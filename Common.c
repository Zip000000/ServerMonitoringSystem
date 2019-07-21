int get_conf_value(const char *file, const char *key, char *val) {
    if (key == NULL || val == NULL) {printf("Wrong parameters\n"); return -1;}
    
    FILE *fp = fopen(file, "r");
    if (fp == NULL) {perror("fopen"); return -1;}
    
    int readnum;
    char *line= (char *)malloc(sizeof(char) * 100);
    size_t n;

    while ((readnum = getline(&line, &n, fp)) != -1) {
        char *p = strstr(line, key);
        if (p == NULL) continue;
        int len = strlen(key);
        if(p[len] != '=') continue;
        strncpy(val, p+len+1, (int)readnum - len - 2);
        break;
    }
    if(readnum == 0) {
        printf("%s Not Found!\n", key);
        free(line);
        return 1;
    }
    return 0;
}


