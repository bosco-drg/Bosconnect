SET(SOURCES ui_home.c
    ui_commandes.c
    ui_donnees.c
    ui_Reglages.c
    ui_Screen1.c
    ui_wifi.c
    ui_wifi1.c
    ui.c
    ui_comp_hook.c
    ui_helpers.c
    ui_events.c)

add_library(ui ${SOURCES})
