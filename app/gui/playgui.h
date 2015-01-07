#ifndef PLAYGUI_H
#define PLAYGUI_H

class Widget;

void FillPlay();
void BuildMenu_OpenPage1();
void BuildMenu_OpenPage2();
void BuildMenu_OpenPage3();
void Click_RightMenu_BackToOpener();
void UpdResTicker();
void ShowMessage(const RichText& msg);
void Resize_Window(Widget* thisw);
void Resize_BuildPreview(Widget* thisw);

#endif
