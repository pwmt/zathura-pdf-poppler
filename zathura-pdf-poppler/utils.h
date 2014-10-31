/* See LICENSE file for license and copyright information */

#ifndef UTILS_H
#define UTILS_H

#include "plugin.h"

zathura_error_t poppler_action_to_zathura_action(PopplerDocument*
    poppler_document, PopplerAction* poppler_action, zathura_action_t** action);

#endif // UTILS_H
