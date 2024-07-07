/* SPDX-License-Identifier: Zlib */

#include "utils.h"

zathura_link_t* poppler_link_to_zathura_link(PopplerDocument* poppler_document, PopplerAction* poppler_action,
                                             zathura_rectangle_t position) {
  zathura_link_type_t type     = ZATHURA_LINK_INVALID;
  zathura_link_target_t target = {ZATHURA_LINK_DESTINATION_UNKNOWN, NULL, 0, -1, -1, -1, -1, 0};

  /* extract link */
  switch (poppler_action->type) {
  case POPPLER_ACTION_NONE:
    type = ZATHURA_LINK_NONE;
    break;
  case POPPLER_ACTION_GOTO_DEST: {
    PopplerDest* poppler_destination = poppler_action->goto_dest.dest;
    if (poppler_destination == NULL) {
      return NULL;
    }

    type = ZATHURA_LINK_GOTO_DEST;

    if (poppler_action->goto_dest.dest->type == POPPLER_DEST_NAMED) {
      poppler_destination = poppler_document_find_dest(poppler_document, poppler_destination->named_dest);
      if (poppler_destination == NULL) {
        return NULL;
      }
    }

    PopplerPage* poppler_page = poppler_document_get_page(poppler_document, poppler_destination->page_num - 1);
    double height             = 0;
    poppler_page_get_size(poppler_page, NULL, &height);

    switch (poppler_destination->type) {
    case POPPLER_DEST_XYZ:
      target.destination_type = ZATHURA_LINK_DESTINATION_XYZ;
      target.page_number      = poppler_destination->page_num - 1;
      if (poppler_destination->change_zoom != 0) {
        target.zoom = poppler_destination->zoom;
      }
      if (poppler_destination->change_left != 0) {
        target.left = poppler_destination->left;
      }
      if (poppler_destination->change_top != 0) {
        target.top = height - MIN(height, poppler_destination->top);
      }
      break;
    case POPPLER_DEST_FIT:
      target.destination_type = ZATHURA_LINK_DESTINATION_FIT;
      target.page_number      = poppler_destination->page_num - 1;
      break;
    case POPPLER_DEST_FITH:
      target.destination_type = ZATHURA_LINK_DESTINATION_FITH;
      target.page_number      = poppler_destination->page_num - 1;
      if (poppler_destination->change_top != 0) {
        target.top = height - MIN(height, poppler_destination->top);
      }
      break;
    case POPPLER_DEST_FITV:
      target.destination_type = ZATHURA_LINK_DESTINATION_FITV;
      target.page_number      = poppler_destination->page_num - 1;
      if (poppler_destination->change_left != 0) {
        target.left = poppler_destination->left;
      }
      break;
    case POPPLER_DEST_FITR:
      target.destination_type = ZATHURA_LINK_DESTINATION_FITR;
      target.page_number      = poppler_destination->page_num - 1;
      if (poppler_destination->change_left != 0) {
        target.left = poppler_destination->left;
      }
      if (poppler_destination->change_top != 0) {
        target.top = height - MIN(height, poppler_destination->top);
      }
      target.right  = poppler_destination->right;
      target.bottom = height - MIN(height, poppler_destination->bottom);
      break;
    case POPPLER_DEST_FITB:
      target.destination_type = ZATHURA_LINK_DESTINATION_FITB;
      target.page_number      = poppler_destination->page_num - 1;
      break;
    case POPPLER_DEST_FITBH:
      target.destination_type = ZATHURA_LINK_DESTINATION_FITBH;
      target.page_number      = poppler_destination->page_num - 1;
      if (poppler_destination->change_top != 0) {
        target.top = height - MIN(height, poppler_destination->top);
      }
      break;
    case POPPLER_DEST_FITBV:
      target.destination_type = ZATHURA_LINK_DESTINATION_FITBV;
      target.page_number      = poppler_destination->page_num - 1;
      target.left             = poppler_destination->top;
      break;
    case POPPLER_DEST_UNKNOWN:
      target.destination_type = ZATHURA_LINK_DESTINATION_UNKNOWN;
      target.page_number      = poppler_destination->page_num - 1;
      break;
    default:
      return NULL;
    }
    break;
  }
  case POPPLER_ACTION_GOTO_REMOTE:
    if (poppler_action->goto_remote.file_name == NULL) {
      return NULL;
    }
    type         = ZATHURA_LINK_GOTO_REMOTE;
    target.value = poppler_action->goto_remote.file_name;
    break;
  case POPPLER_ACTION_URI:
    type         = ZATHURA_LINK_URI;
    target.value = poppler_action->uri.uri;
    break;
  case POPPLER_ACTION_LAUNCH:
    type         = ZATHURA_LINK_LAUNCH;
    target.value = poppler_action->launch.file_name;
    break;
  case POPPLER_ACTION_NAMED:
    type         = ZATHURA_LINK_NAMED;
    target.value = poppler_action->named.named_dest;
    break;
  default:
    return NULL;
  }

  return zathura_link_new(type, position, target);
}
