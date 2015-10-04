#ifndef MAIN_H
#define MAIN_H

#include <cstdio>
#include <QApplication>
#include <QTranslator>
std::FILE *fopenQ(const QString& name,const char* mode);

extern QTranslator translator;
extern QApplication* pApp;

#endif // MAIN_H

