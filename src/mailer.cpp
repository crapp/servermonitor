#include "mailer.h"

Mailer::Mailer(string mailSubject, string mailMessage) : mailSubject(mailSubject),
    mailMessage(mailMessage)
{
    this->cfg = boost::make_shared<Config>();
}

