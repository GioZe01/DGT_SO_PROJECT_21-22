/**
 * \file boolean.h
 * \brief Type Definition for the boolean type using integer values
 * Done for C89 compatibility
 * \author Giovanni Terzuolo
 */
#ifndef DGT_SO_PROJECT_21_22_BOOLEAN_H
#define DGT_SO_PROJECT_21_22_BOOLEAN_H
/* Could be implemented using bitwise operator to save space, creating a single boolean var
 * that can hold 64 boolean variable copressed in only one
 *
 * -> creating a method to che if it is true or not by getting position name and applying the mask
 * */
#define TRUE 1 /**< The value of the boolean true */
#define FALSE 0 /**< The value of the boolean false */
typedef int Bool; /**< The type definition for the boolean type */

char *bool_to_string(Bool b); /**< Convert the boolean value to a string */

#endif /*DGT_SO_PROJECT_21_22_BOOLEAN_H*/
