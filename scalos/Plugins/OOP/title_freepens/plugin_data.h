// plugin_data.h
// $Date$
// $Revision$

#ifndef PLUGIN_DATA_H_INCLUDED
#define PLUGIN_DATA_H_INCLUDED

#include <intuition/classusr.h>
#include <scalos/scalos.h>

#include <defs.h>
#include <Year.h>

#define PLUGIN_TYPE		OOP

#define LIB_VERSION		40		/* Version of our plugin */
#define LIB_REVISION		3		/* Revision of our plugin */
#define NAME			"title_freepens"/* Name of our plugin (must be the same as the executable filename minus the .plugin [gets added later]) */
#define LIBVER			STR(LIB_VERSION) "." STR(LIB_REVISION)	       /* part of the version string for our plugin */

#define LIB_NAME 		NAME ".plugin"
#define LIB_IDSTRING 		NAME LIBVER " " COMPILER_STRING
#define LIB_VERSTRING 		"\0$VER: " LIB_NAME " " LIBVER " (07.11.2009) " COMPILER_STRING

#define CI_CLASSNAME 		"Title.sca"
#define CI_SUPERCLASSNAME 	"Title.sca"
#define CI_PLUGINNAME 		"Title FreePens"
#define CI_DESCRIPTION 		"Adds a %wp function to the screentitle to " \
				"show all free pens. " COMPILER_STRING
#define CI_AUTHOR 		"David McMinn, Stefan Sommerfield"

#define CI_HOOKFUNC		FreePens

#ifndef __amigaos4__
#define NOINITPLUGIN
#define NOCLOSEPLUGIN
#endif

//----------------------------------------------------------------------------
// code and includes to define the structs and functions used above

#endif /* PLUGUN_DATA_H_INCLUDED */

