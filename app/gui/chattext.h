

#ifndef CHATTEXT_H
#define CHATTEXT_H

#define CHAT_LINES		(100/8)

struct Widget;
struct ViewLayer;
struct RichText;

void AddChat(ViewLayer* playview);
void AddChat(RichText* newl);

#endif