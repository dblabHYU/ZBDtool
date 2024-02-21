#include <stdio.h>
#include <stdlib.h>
#include <libzbd/zbd.h>
#include "/home/libzbd/tools/cli/zbd.h"

void print_padded(const char *str, int width, int cmd) {
    int len = strlen(str);
    int padding = width - len;
    for (int i = 0; i < padding; i++) {
        printf(" ");
    }
    switch (cmd) {
        case 0:
            printf("%s", str);
            break;
        case 1:
            printf("\033[0;31m%s\033[0m", str);
            break;
        case 2:
            printf("\033[0;32m%s\033[0m", str);
            break;
        default:
            printf("%s", str);
    }
}


void print_cmd(char* cmd_list, int idx, struct zbd_opts *opts, unsigned int nr_zones, int length, struct zbd_zone *zones) {
    printf("|");
    char size_len[64];
    size_t length3;
    switch (idx) {
        case 0:
            length3 = sprintf(size_len, "%s", opts->dev_info.vendor_id);
            break;
        case 1:
            length3 = sprintf(size_len, "%s", zbd_device_model_str(opts->dev_info.model, false));
            break;
        case 2:
            length3 = sprintf(size_len, "%0.03F GB", (double)(opts->dev_info.nr_sectors << 9) / 1000000000);
            break;
        case 3:
            length3 = sprintf(size_len, "%lld MB", zbd_zone_len(&zones[0])/1024/1024*nr_zones);
            break;
        case 4:
            length3 = sprintf(size_len, "%lld MB", zbd_zone_len(&zones[0])/1024/1024);
            break;
        case 5:
            if (opts->dev_info.max_nr_open_zones == 0)
                length3 = sprintf(size_len, "%s", "no limit");
            else
                length3 = sprintf(size_len, "%ls", &opts->dev_info.max_nr_open_zones);
            break;
        case 6:
            if (opts->dev_info.max_nr_active_zones == 0)
                length3 = sprintf(size_len, "%s", "no limit");
            else
                length3 = sprintf(size_len, "%ls", &opts->dev_info.max_nr_active_zones);
            break;
    }
    print_padded(cmd_list, length/2, 0);
    printf("%s", size_len);
    print_padded("", length - length/2-length3 - 2, 0);
    printf("|\n");
}


int print_zone_info_cmd(int cmd, int zone_num, struct zbd_zone *zones, char *size_len)
{
    switch (cmd) {
        case 0:
            return sprintf(size_len, "Zone %d", zone_num);
        case 1:
            return sprintf(size_len, "ofst %014llu", zbd_zone_start(&zones[zone_num]));
        case 2:
            if (!strcmp(zbd_zone_cond_str(&zones[zone_num], true),"fu"))
                return sprintf(size_len, "wp %014llu", zbd_zone_start(&zones[zone_num])+zbd_zone_capacity(&zones[zone_num])-1);
            else
                return sprintf(size_len, "wp %014llu", zbd_zone_wp(&zones[zone_num]));
        case 3:
            return sprintf(size_len, "%s", zbd_zone_cond_str(&zones[zone_num], true));
        case 4:
            if (!strcmp(zbd_zone_cond_str(&zones[zone_num], true),"fu"))
                return sprintf(size_len, "util 100%%");
            else
                return sprintf(size_len, "util %0.02f%%", ((float)zbd_zone_wp(&zones[zone_num]) - (float)zbd_zone_start(&zones[zone_num])) / (float)zbd_zone_capacity(&zones[zone_num])*100);
        default:
            return -1;
    }
}

void print_zone(unsigned int nr_zones, int cmd, int zone_num, struct zbd_zone *zones, int length)
{
    printf("|");
    for (int j = 0; j < 4 && zone_num+j < nr_zones; ++j)
    {
        if (zbd_zone_seq(&zones[zone_num+j])) {
            if (!strcmp(zbd_zone_cond_str(&zones[zone_num+j], true),"fu"))
            {
                char size_len[64];
                int length3;
                length3 = print_zone_info_cmd(cmd, zone_num+j, zones, size_len);
                print_padded(size_len, length/2+length3/2, 1);
                print_padded("", length - length/2-length3/2 - 1, 1);
            }
            else if (!strcmp(zbd_zone_cond_str(&zones[zone_num+j], true),"oi") || !strcmp(zbd_zone_cond_str(&zones[zone_num+j], true),"oe"))
            {
                char size_len[64];
                int length3;
                length3 = print_zone_info_cmd(cmd, zone_num+j, zones, size_len);
                print_padded(size_len, length/2+length3/2, 2);
                print_padded("", length - length/2-length3/2 - 1, 2);
            }
            else
            {
                char size_len[64];
                int length3;
                length3 = print_zone_info_cmd(cmd, zone_num+j, zones, size_len);
                print_padded(size_len, length/2+length3/2, 0);
                print_padded("", length - length/2-length3/2 - 1, 0);
            }
        }
        printf("|");
    }
    printf("\n");
}


void print_zone_info(unsigned int nr_zones, struct zbd_zone *zones, int length)
{
    for (int i = 0; i < nr_zones; i += 4)
    {
        for (int j = 0; j < 5; j++)
            print_zone(nr_zones, j, i, zones, length);
        printf("+---------------------------------------------------------------------------------------------------------------+\n");
    }

}


int main() {
    unsigned int nr_zones;
    ssize_t ret;
    struct zbd_zone *zones;
    struct zbd_opts opts;
    char* cmd_list[7] = {"Vendor ID : ", "Zone model : ", "Capacity : ", "Total zone size : ", "Each zone size : ", "Maximum number of open zones : ", "Maximum number of active zones : "};
    memset(&opts, 0, sizeof(struct zbd_opts));
	opts.rep_opt = ZBD_RO_ALL;
	opts.rep_dump = false;
	opts.unit = 1;
    opts.cmd = ZBD_REPORT;

    ret = zbd_open("/dev/nvme0n1", ZBD_RO_ALL, &opts.dev_info);
    if (ret < 0) {
        fprintf(stderr, "Failed to open device\n");
        return ret;
    }

    nr_zones = opts.dev_info.nr_zones;
    zones = (struct zbd_zone *)malloc(nr_zones * sizeof(struct zbd_zone));
    if (!zones) {
        fprintf(stderr, "Failed to allocate memory for zone information\n");
        zbd_close(ret);
        return -1;
    }
    ret = zbd_report_zones(ret, 0, 0, ZBD_RO_ALL, zones, &nr_zones);
    size_t length = snprintf(NULL, 0, "*-------------------------------------------------- %u Zones ---------------------------------------------------*", nr_zones);
    printf("+-------------------------------------------------- %u Zones ---------------------------------------------------+\n", nr_zones);
    for (int i = 0; i < 7; i++)
    {
        print_cmd(cmd_list[i], i, &opts, nr_zones, length, &zones[0]);
    }
    printf("+---------------------------------------------------------------------------------------------------------------+\n");
    print_zone_info(nr_zones, zones, (length)/4);


    free(zones);
    zbd_close(ret);

    return 0;
}