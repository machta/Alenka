#ifndef LOCALEOVERRIDE_H
#define LOCALEOVERRIDE_H

#include <functional>
#include <locale>

class LocaleOverride {
public:
  static void executeWithCLocale(const std::function<void()> &code) {
    const std::locale currentLocale;
    std::locale::global(std::locale("C"));

    code();

    // For some reason this is effective at eliminating some of the parsing
    // problems (e.g. in clFFT or libeep_read) even though currentLocale seems
    // to already be the desired "C" locale.
    // So there is probably no reason to set the global to what it was before,
    // but I will leave it like it is now unless it breaks something.
    std::locale::global(currentLocale);
  }
};

#endif // LOCALEOVERRIDE_H
