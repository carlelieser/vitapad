#ifndef __MENU_H__
#define __MENU_H__

#include "settings.h"

extern char *MENU_ITEMS[5];
extern int CURRENT_MENU_ITEM;

extern int menuItemSelected();
extern int getMenuItemIndex();
extern void setCurrentMenuItem();
extern void displayMenu();
extern void handleMenuActions();

#define MAX_MENU_ITEM_TITLE_LENGTH 140


#endif