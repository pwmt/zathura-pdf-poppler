/* See LICENSE file for license and copyright information */

#include "utils.h"
#include "macros.h"

zathura_error_t
poppler_action_to_zathura_action(PopplerDocument* poppler_document, PopplerAction* poppler_action, zathura_action_t** action)
{
  if (poppler_document == NULL || poppler_action == NULL || action == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  zathura_action_type_t type = ZATHURA_ACTION_UNKNOWN;

  /* ZATHURA_ACTION_UNKNOWN, #<{(|*< Invalid type |)}># */
  /* ZATHURA_ACTION_NONE, #<{(|*< No action |)}># */
  /* ZATHURA_ACTION_GOTO, #<{(|*< actions to a page |)}># */
  /* ZATHURA_ACTION_GOTO_REMOTE, #<{(|*< actions to a page |)}># */
  /* ZATHURA_ACTION_GOTO_EMBEDDED, #<{(|*< actions to an embedded file |)}># */
  /* ZATHURA_ACTION_LAUNCH, #<{(|*< actions to an external source |)}># */
  /* ZATHURA_ACTION_THREAD, #<{(|*< Begin reading an article |)}># */
  /* ZATHURA_ACTION_URI, #<{(|*< actions to an external source |)}># */
  /* ZATHURA_ACTION_SOUND, #<{(|*< Play a sound |)}># */
  /* ZATHURA_ACTION_MOVIE, #<{(|<< Play a movie |)}># */
  /* ZATHURA_ACTION_HIDE_ANNOTATIONS, #<{(|*< Set an annotation's hidden flag |)}># */
  /* ZATHURA_ACTION_NAMED, #<{(|*< actions to an external source |)}># */
  /* ZATHURA_ACTION_SET_OCG_STATE, #<{(|*< Sets the states of optional content groups |)}># */
  /* ZATHURA_ACTION_RENDITION, #<{(|*< Controls the playing of multimedia content. |)}># */
  /* ZATHURA_ACTION_TRANSITION, #<{(|*< Updates the display of a document, using a transition diary |)}># */
  /* ZATHURA_ACTION_GOTO_3D_VIEW #<{(|*< Set the current view of a 3d annotation |)}># */

  switch (poppler_action->type) {
    case POPPLER_ACTION_UNKNOWN:
      type = ZATHURA_ACTION_UNKNOWN;
      break;
    case POPPLER_ACTION_NONE:
      type = ZATHURA_ACTION_NONE;
      break;
    case POPPLER_ACTION_GOTO_DEST:
      type = ZATHURA_ACTION_GOTO_REMOTE;
      break;
    case POPPLER_ACTION_GOTO_REMOTE:
      type = ZATHURA_ACTION_GOTO_REMOTE;
      break;
    case POPPLER_ACTION_LAUNCH:
      type = ZATHURA_ACTION_LAUNCH;
      break;
    case POPPLER_ACTION_URI:
      type = ZATHURA_ACTION_URI;
      break;
    case POPPLER_ACTION_NAMED:
      type = ZATHURA_ACTION_NAMED;
      break;
    case POPPLER_ACTION_MOVIE:
      type = ZATHURA_ACTION_MOVIE;
      break;
    case POPPLER_ACTION_RENDITION:
      type = ZATHURA_ACTION_RENDITION;
      break;
    case POPPLER_ACTION_OCG_STATE:
      type = ZATHURA_ACTION_SET_OCG_STATE;
      break;
    case POPPLER_ACTION_JAVASCRIPT:
      type = ZATHURA_ACTION_JAVASCRIPT;
      break;
    default:
      type = ZATHURA_ACTION_UNKNOWN;
      break;
  }

  if (type == ZATHURA_ACTION_UNKNOWN) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  return zathura_action_new(action, type);
}

zathura_error_t
pdf_attachment_save(zathura_attachment_t* UNUSED(attachment), const char* path, void* user_data)
{
  if (user_data == NULL || path == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  if (poppler_attachment_save((PopplerAttachment*) user_data, path, NULL) ==
      FALSE) {
    return ZATHURA_ERROR_UNKNOWN;
  }

  return ZATHURA_ERROR_OK;
}
