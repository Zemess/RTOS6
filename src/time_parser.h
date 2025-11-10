#pragma once

// Virhekoodit aikaparserille
#define TP_ERR_NULL    -1
#define TP_ERR_LEN     -2
#define TP_ERR_NAN     -3
#define TP_ERR_RANGEH  -4
#define TP_ERR_RANGEM  -5
#define TP_ERR_RANGES  -6
#define TP_ERR_ZERO    -7

int time_parse(const char *hhmmss);
