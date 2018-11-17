#include <string.h>
#include "DApplication.hpp"

int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);

    PDApplication pApp = new CDApplication("~/.SteamCAD/");

    gtk_main();

    delete pApp;
    return 0;
}

