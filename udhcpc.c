#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <net/if.h>
#include "QMIThread.h"

static int ql_system(const char *shell_cmd) {
    int ret = 0;
    dbg_time("%s", shell_cmd);
    ret = system(shell_cmd);
    if (ret) {
        //dbg_time("Fail to system(\"%s\") = %d, errno: %d (%s)", shell_cmd, ret, errno, strerror(errno));
    }
    return ret;
}

static void ql_set_mtu(const char *ifname, int ifru_mtu) {
    int inet_sock;
    struct ifreq ifr;

    inet_sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (inet_sock > 0) {
        strcpy(ifr.ifr_name, ifname);

        if (!ioctl(inet_sock, SIOCGIFMTU, &ifr)) {
            if (ifr.ifr_ifru.ifru_mtu != ifru_mtu) {
                dbg_time("change mtu %d -> %d", ifr.ifr_ifru.ifru_mtu , ifru_mtu);
                ifr.ifr_ifru.ifru_mtu = ifru_mtu;
                ioctl(inet_sock, SIOCSIFMTU, &ifr);
            }
        }

        close(inet_sock);
    }
}

static void* udhcpc_thread_function(void* arg) {
    FILE * udhcpc_fp;
    char *udhcpc_cmd = (char *)arg;

    if (udhcpc_cmd == NULL)
        return NULL;

    dbg_time("%s", udhcpc_cmd);
    udhcpc_fp = popen(udhcpc_cmd, "r");
    free(udhcpc_cmd);
    if (udhcpc_fp) {
        char buf[0xff];

        buf[sizeof(buf)-1] = '\0';
        while((fgets(buf, sizeof(buf)-1, udhcpc_fp)) != NULL) {
            if ((strlen(buf) > 1) && (buf[strlen(buf) - 1] == '\n'))
                buf[strlen(buf) - 1] = '\0';
            dbg_time("%s", buf);
        }

        pclose(udhcpc_fp);
    }

    return NULL;
}

#define USE_DHCLIENT
#ifdef USE_DHCLIENT
static int dhclient_alive = 0;
#endif
static int dibbler_client_alive = 0;

void udhcpc_start(PROFILE_T *profile) {
    char *ifname = profile->usbnet_adapter;
    char shell_cmd[128];

    if (profile->qmapnet_adapter) {
        ifname = profile->qmapnet_adapter;
    }

    if (profile->rawIP && profile->ipv4.Address && profile->ipv4.Mtu) {
        ql_set_mtu(ifname, (profile->ipv4.Mtu));
    }

    if (strcmp(ifname, profile->usbnet_adapter)) {
        snprintf(shell_cmd, sizeof(shell_cmd) - 1, "ifconfig %s up", profile->usbnet_adapter);
        ql_system(shell_cmd);
    }

    snprintf(shell_cmd, sizeof(shell_cmd) - 1, "ifconfig %s up", ifname);
    ql_system(shell_cmd);

    char udhcpc_cmd[128];
    pthread_attr_t udhcpc_thread_attr;
    pthread_t udhcpc_thread_id;

    pthread_attr_init(&udhcpc_thread_attr);
    pthread_attr_setdetachstate(&udhcpc_thread_attr, PTHREAD_CREATE_DETACHED);

    if (profile->ipv4.Address) {
#ifdef USE_DHCLIENT
        snprintf(udhcpc_cmd, sizeof(udhcpc_cmd), "dhclient -4 -d -q --no-pid %s", ifname);
        dhclient_alive++;
#endif
        pthread_create(&udhcpc_thread_id, &udhcpc_thread_attr, udhcpc_thread_function, (void*)strdup(udhcpc_cmd));
        sleep(1);
    }

    if (profile->ipv6.Address[0] && profile->ipv6.PrefixLengthIPAddr) {
#ifdef USE_DHCLIENT
        snprintf(udhcpc_cmd, sizeof(udhcpc_cmd), "dhclient -6 -d -q --no-pid %s",  ifname);
        dhclient_alive++;
#endif

        pthread_create(&udhcpc_thread_id, &udhcpc_thread_attr, udhcpc_thread_function, (void*)strdup(udhcpc_cmd));
    }
}

void udhcpc_stop(PROFILE_T *profile) {
    char *ifname = profile->usbnet_adapter;
    char shell_cmd[128];
    char reset_ip[128];

    if (profile->qmapnet_adapter) {
        ifname = profile->qmapnet_adapter;
    }

#ifdef USE_DHCLIENT
    if (dhclient_alive) {
        system("killall dhclient");
        dhclient_alive = 0;
    }
#endif

    if (dibbler_client_alive) {
        system("killall dibbler-client");
        dibbler_client_alive = 0;
    }
    snprintf(shell_cmd, sizeof(shell_cmd) - 1, "ifconfig %s down", ifname);

    ql_system(shell_cmd);
    snprintf(reset_ip, sizeof(reset_ip) - 1, "ifconfig %s 0.0.0.0", ifname);
    ql_system(reset_ip);
}
