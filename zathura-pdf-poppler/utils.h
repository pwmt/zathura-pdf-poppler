/* See LICENSE file for license and copyright information */

#ifndef UTILS_H
#define UTILS_H

#include "plugin.h"

#define GFOREACH(item, list) for(GList *__glist = list; __glist && (item = __glist->data, true); __glist = __glist->next)

#define LENGTH(x) (sizeof(x)/sizeof((x)[0]))

zathura_error_t poppler_action_to_zathura_action(PopplerDocument*
    poppler_document, PopplerAction* poppler_action, zathura_action_t** action);

#endif // UTILS_H
