SET(SOURCES ui_home.c
    ui_firstpage.c
    ui_commandes.c
    ui_donnees.c
    ui_Wifi1.c
    ui_Wifi2.c
    ui_User1.c
    ui_User2.c
    ui_Reglages.c
    ui_Screen1.c
    ui.c
    ui_comp_hook.c
    ui_helpers.c
    ui_events.c)

add_library(ui ${SOURCES})
