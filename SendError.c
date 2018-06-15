#include "SendError.h"

void sendError(HTTPResponse *v, int errCode, const char *errTxt, const char *htmlType) {
  v->code     = errCode;
  v->codeText = errTxt;
  v->conType  = htmlType;
  v->conLen   = 0;
  v->lastMod  = NULL;
}
