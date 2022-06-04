
#ifndef DGT_SO_PROJECT_21_22_BOOLEAN_H
#define DGT_SO_PROJECT_21_22_BOOLEAN_H
/* Could be implemented using bitwise operator to save space, creating a single boolean var
 * that can hold 64 boolean variable copressed in only one
 *
 * -> creating a method to che if it is true or not by getting position name and applying the mask
 * */
#define TRUE 1
#define FALSE 0
typedef  int Bool;
char *bool_to_string(Bool b);
#endif /*DGT_SO_PROJECT_21_22_BOOLEAN_H*/
