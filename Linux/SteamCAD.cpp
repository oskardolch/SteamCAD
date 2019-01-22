#include <string.h>
#include <libintl.h>
#include <locale.h>
#include "DApplication.hpp"
#ifdef CONFDIR
#include "config.h"
#endif

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "");
#ifdef LOCALEDIR
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);
#endif
    //bindtextdomain("SteamCAD", "/home/pavel/Projects/SteamCAD/Linux/po");
    //bind_textdomain_codeset("SteamCAD", "UTF-8");
    //textdomain("SteamCAD");

    gtk_init(&argc, &argv);

    //gtk_init_with_args(&argc, &argv, "SteamCAD", NULL, "steamcad", NULL);

#ifdef CONFDIR
    PDApplication pApp = new CDApplication(CONFDIR, "~/.SteamCAD/");
#else
    PDApplication pApp = new CDApplication(NULL, "~/.SteamCAD/");
#endif

    gtk_main();

    delete pApp;
    return 0;
}

