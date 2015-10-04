#ifndef MAIN_H
#define MAIN_H

#include <cstdio>
#include <QApplication>
#include <QTranslator>
std::FILE *fopenQ(const QString& name,const QString& mode);

extern QTranslator translator;
extern QApplication* pApp;

#endif // MAIN_H

