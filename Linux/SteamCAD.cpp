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
    //bind_textdomain_codeset(PACKAGE, "UTF-8");
    textdomain(PACKAGE);
#endif

    gtk_init(&argc, &argv);

#ifdef CONFDIR
    PDApplication pApp = new CDApplication(CONFDIR);
#else
    PDApplication pApp = new CDApplication(NULL);
#endif

    gtk_main();

    delete pApp;
    return 0;
}

