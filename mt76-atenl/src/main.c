/* Copyright (C) 2021-2022 Mediatek Inc. */

#include <signal.h>
#include <sys/select.h>
#include <sys/wait.h>
#include "atenl.h"

static const char *progname;
bool atenl_enable;

void sig_handler(int signum)
{
	atenl_enable = false;
}

void atenl_init_signals()
{
	if (signal(SIGINT, sig_handler) == SIG_ERR)
		goto out;
	if (signal(SIGTERM, sig_handler) == SIG_ERR)
		goto out;
	if (signal(SIGABRT, sig_handler) == SIG_ERR)
		goto out;
	if (signal(SIGUSR1, sig_handler) == SIG_ERR)
		goto out;
	if (signal(SIGUSR2, sig_handler) == SIG_ERR)
		goto out;

	return;
out:
	perror("signal");
}

static int phy_lookup_idx(const char *name)
{
	char buf[128];
	FILE *f;
	size_t len;
	int ret;

	ret = snprintf(buf, sizeof(buf), "/sys/class/ieee80211/%s/index", name);
	if (snprintf_error(sizeof(buf), ret))
		return -1;

	f = fopen(buf, "r");
	if (!f)
		return -1;

	len = fread(buf, 1, sizeof(buf) - 1, f);
	fclose(f);

	if (!len)
		return -1;

	buf[len] = 0;
	return atoi(buf);
}

static void usage(void)
{
	fprintf(stderr, "Usage:\n");
	printf("  %s [-u] [-i phyX]\n", progname);
	printf("options:\n"
	       "  -h = show help text\n"
	       "  -i = phy name of driver interface, please use first phy for dbdc\n"
	       "  -u = use unicast to respond to HQADLL\n");
	printf("examples:\n"
	       "  %s -u -i phy0\n", progname);

	exit(EXIT_FAILURE);
}

static int atenl_parent_work(struct atenl *an)
{
	int sock_eth = an->sock_eth;
	int count, ret = 0;
	fd_set readfds;

	atenl_info("[%d]%s: start for receiving HQA commands\n", getpid(), __func__);

	while (atenl_enable) {
		FD_ZERO(&readfds);
		FD_SET(sock_eth, &readfds);
		count = select(sock_eth + 1, &readfds, NULL, NULL, NULL);

		if (count < 0) {
			atenl_err("%s: select failed, %s\n", __func__, strerror(errno));
			continue;
		} else if (count == 0) {
			usleep(1000);
			continue;
		} else {
			if (FD_ISSET(sock_eth, &readfds)) {
				struct atenl_data *data = calloc(1, sizeof(struct atenl_data));

				ret = atenl_eth_recv(an, data);
				if (ret) {
					kill(an->child_pid, SIGUSR1);
					return ret;
				}

				ret = atenl_hqa_recv(an, data);
				if (ret < 0) {
					kill(an->child_pid, SIGUSR1);
					return ret;
				}

				free(data);
			}
		}
	}

	atenl_info("[%d]%s: parent work end\n", getpid(), __func__);

	return ret;
}

static int atenl_child_work(struct atenl *an)
{
	int rfd = an->pipefd[PIPE_READ], count;
	int ret = 0;
	fd_set readfds;

	atenl_info("[%d]%s: start for sending back results\n", getpid(), __func__);

	while (atenl_enable) {
		struct atenl_data *data = calloc(1, sizeof(struct atenl_data));

		FD_ZERO(&readfds);
		FD_SET(rfd, &readfds);

		count = select(FD_SETSIZE, &readfds, NULL, NULL, NULL);

		if (count < 0) {
			atenl_err("%s: select failed, %s\n", __func__, strerror(errno));
			continue;
		} else if (count == 0) {
			usleep(1000);
			continue;
		} else {
			if (FD_ISSET(rfd, &readfds)) {
				count = read(rfd, data->buf, RACFG_PKT_MAX_SIZE);
				atenl_dbg("[%d]PIPE Read %d bytes\n", getpid(), count);

				if (count < 0) {
					atenl_info("%s: %s\n", __func__, strerror(errno));
				} else if (count == 0) {
					continue;
				} else {
					int ret;

					ret = atenl_hqa_proc_cmd(an, data);
					if (ret) {
						kill(getppid(), SIGUSR2);
						goto out;
					}

					ret = atenl_eth_send(an, data);
					if (ret) {
						kill(getppid(), SIGUSR2);
						goto out;
					}
				}
			}
		}
	}

out:
	atenl_info("[%d]%s: child work end\n", getpid(), __func__);

	return ret;
}

int main(int argc, char **argv)
{
	int opt, phy_idx, ret = 0;
	char *phy = "phy0", *cmd = NULL;
	struct atenl *an;

	progname = argv[0];

	an = calloc(1, sizeof(struct atenl));
	if (!an)
		return -ENOMEM;

	while(1) {
		opt = getopt(argc, argv, "hi:uc:");
		if (opt == -1)
			break;

		switch (opt) {
			case 'h':
				usage();
				free(an);
				return 0;
			case 'i':
				phy = optarg;
				break;
			case 'u':
				an->unicast = true;
				printf("Opt: use unicast to send response\n");
				break;
			case 'c':
				cmd = optarg;
				break;
			default:
				fprintf(stderr, "Not supported option\n");
				break;
		}
	}

	phy_idx = phy_lookup_idx(phy);
	if (phy_idx < 0 || phy_idx > UCHAR_MAX) {
		fprintf(stderr, "Could not find phy '%s'\n", phy);
		free(an);
		return 2;
	}

	if (cmd) {
		atenl_eeprom_cmd_handler(an, phy_idx, cmd);
		goto out;
	}

	atenl_enable = true;
	atenl_init_signals();

	/* background ourself */
	if (!fork()) {
		pid_t pid;

		ret = atenl_eeprom_init(an, phy_idx);
		if (ret)
			goto out;

		ret = atenl_eth_init(an);
		if (ret)
			goto out;

		ret = pipe(an->pipefd);
		if (ret) {
			perror("Pipe");
			goto out;
		}

		pid = fork();
		an->child_pid = pid;
		if (pid < 0) {
			perror("Fork");
			ret = pid;
			goto out;
		} else if (pid == 0) {
			close(an->pipefd[PIPE_WRITE]);
			atenl_child_work(an);
		} else {
			int status;

			close(an->pipefd[PIPE_READ]);
			atenl_parent_work(an);
			waitpid(pid, &status, 0);
		}
	} else {
		usleep(800000);
	}

	ret = 0;
out:
	if (an->sock_eth)
		close(an->sock_eth);
	if (an->eeprom_fd || an->eeprom_data)
		atenl_eeprom_close(an);
	free(an);

	return ret;
}
