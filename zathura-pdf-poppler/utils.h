/* See LICENSE file for license and copyright information */

#ifndef UTILS_H
#define UTILS_H

#include "plugin.h"

/**
 * Convert a poppler link object to a zathura link object
 *
 * @param poppler_document The poppler document
 * @param poppler_action The poppler action
 * @param position The position of the link
 *
 * @return Zathura link object 
 */
GIRARA_HIDDEN zathura_link_t* poppler_link_to_zathura_link(PopplerDocument* poppler_document,
    PopplerAction* poppler_action, zathura_rectangle_t position);

#endif // UTILS_H
