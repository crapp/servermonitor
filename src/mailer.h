#ifndef MAILER_H
#define MAILER_H

#include <stdio.h> //needed for popen and FILE handle
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include "config.h"

using namespace std;

class Mailer
{
public:
    Mailer(string mailSubject, string mailMessage);
private:
    const string mailSubject;
    const string mailMessage;
    boost::shared_ptr<Config> cfg;
};

#endif // MAILER_H
