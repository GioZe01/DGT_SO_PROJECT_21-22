
#ifndef DGT_SO_PROJECT_21_22_DEBUG_UTILITY_H
#define DGT_SO_PROJECT_21_22_DEBUG_UTILITY_H
#include "glob.h"
/*  DEBUG_MESSAGE definitions:  */
/*TODO: add DEBUG Line Splitter*/
#ifdef DEBUG
#define DEBUG_BLOCK_ACTION_START(mex) fprintf(stderr, "%s--------------- %s ---------------\n%s",COLOR_RESET_ANSI_CODE, mex, COLOR_RESET_ANSI_CODE);
#define DEBUG_BLOCK_ACTION_END() fprintf(stderr, "%s--------------- END ---------------\n%s",COLOR_RESET_ANSI_CODE,COLOR_RESET_ANSI_CODE);
#define DEBUG_NOTIFY_ACTIVITY_RUNNING(mex) fprintf(stderr,"%s%s [DEBUG-NOTIFY]:= %s\n", COLOR_RESET_ANSI_CODE,COLOR_YELLOW_ANSI_CODE,mex)
#define DEBUG_NOTIFY_ACTIVITY_DONE(mex) fprintf(stderr,"%s%s [DEBUG-NOTIFY]:= %s\n", COLOR_RESET_ANSI_CODE,COLOR_GREEN_ANSI_CODE,mex)
#define DEBUG_MESSAGE(mex) fprintf(stderr, "%s%s [DEBUG]:= | file_in: %s | pid: %d | line %d |\n -> %s\n%s", COLOR_RESET_ANSI_CODE, COLOR_BLUE_ANSI_CODE, __FILE__,getpid(), __LINE__,mex, COLOR_RESET_ANSI_CODE)
#define DEBUG_SIGNAL(mex, signum) fprintf(stderr, "%s%s [DEBUG-SIGNAL]:= | file_in: %s | pid: %d | line %d |\n -> %s, {%d}\n%s",COLOR_RESET_ANSI_CODE, COLOR_YELLOW_ANSI_CODE, __FILE__, getpid(), __LINE__,mex, signum,COLOR_RESET_ANSI_CODE)
#define DEBUG_ERROR_MESSAGE(mex) fprintf(stderr, "%s%s [DEBUG]:= | file_in: %s | pid: %d | line %d |\n -> %s\n%s", COLOR_RESET_ANSI_CODE, COLOR_WHITE_ANSI_CODE, __FILE__, getpid(), __LINE__, mex, COLOR_RESET_ANSI_CODE)
#endif
#endif /*DGT_SO_PROJECT_21_22_DEBUG_UTILITY_H*/
