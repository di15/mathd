

#include "main.h"
#include "gui.h"
#include "menu.h"
#include "unit.h"
#include "building.h"
#include "road.h"
#include "powerline.h"
#include "pipeline.h"
#include "script.h"
#include "chat.h"
#include "image.h"
#include "font.h"
#include "editor.h"
#include "selection.h"
#include "player.h"
#include "save.h"

CTrigger* g_scripthead = NULL;
Hint g_lasthint;

CCondition::CCondition()
{
	type = CONDITION_NONE;
	strcpy(name, "");
	met = false;
	target = 0;
	fval = 0;
	count = 0;
	owner = 0;
}

CEffect::CEffect()
{
	type = EFFECT_NONE;
	strcpy(name, "");
	trigger = NULL;
	imgdw = 100;
	imgdh = 100;
}

CTrigger::CTrigger()
{
	strcpy(name, "");
	prev = NULL;
	next = NULL;
	enabled = true;
}

Hint::Hint()
{
	reset();
}

void Hint::reset()
{
	message = "";
	graphic = "gui\\transp.png";
	gwidth = 100;
	gheight = 100;
}

int CountTriggers()
{
	int numt = 0;
	CTrigger* t = g_scripthead;

	while(t)
	{
		numt++;
		t = t->next;
	}

	return numt;
}

void Click_DelTrigger()
{
	Change_Triggers();
	CTrigger* t = GetChosenTrigger();

	if(t == NULL)
		return;

	CView* v = g_GUI.getview("ed script");
	Widget* lb = v->getwidget("triggers", LISTBOX);
	lb->erase(lb->selected);

	if(t->prev == NULL)
	{
		g_scripthead = t->next;
	}
	else
	{
		t->prev->next = t->next;
	}

	if(t->next != NULL)
	{
		t->next->prev = t->prev;
	}

	delete [] t;

	CTrigger* memloc = t;
	t = g_scripthead;

	while(t)
	{
		for(int i=0; i<t->effects.size(); i++)
		{
			if(t->effects[i].trigger == memloc)
				t->effects[i].trigger = NULL;
		}

		t = t->next;
	}
}

void Click_EditTrigger()
{
	Change_Triggers();
	if(GetChosenTrigger() == NULL)
		return;

	OpenAnotherView("ed trigger");
	ReGUITrigger();
}

void AttachTrigger(CTrigger* before, const char* name)
{
	CTrigger* newt;
	newt = new CTrigger();
	//strcpy(newe->name, name);
	strncpy(newt->name, name, 15);

	if(before == NULL)
	{
		g_scripthead = newt;
	}
	else
	{
		CTrigger* next = before->next;
		newt->next = next;
		before->next = newt;

		if(next != NULL)
		{
			next->prev = newt;
		}

		newt->prev = before;
	}
}

void Click_AddTrigger()
{
	Change_Triggers();
	CView* v = g_GUI.getview("ed script");
	Widget* lb = v->getwidget("triggers", LISTBOX);
	Widget* eb = v->getwidget("newtrigger", EDITBOX);

	if(!UniqueTrigger(eb->value.c_str()))
	{
		OpenAnotherView("name unique");
		return;
	}

	lb->options.push_back(eb->value);
	CTrigger* tail = GetScriptTail();
	AttachTrigger(tail, eb->value.c_str());
}

void Click_AddCondition()
{
	CView* v = g_GUI.getview("ed trigger");
	Widget* lb = v->getwidget("conditions", LISTBOX);
	Widget* eb = v->getwidget("newcondition", EDITBOX);

	if(!UniqueCondition(eb->value.c_str()))
	{
		OpenAnotherView("name unique");
		return;
	}
	
	lb->options.push_back(eb->value);
	CTrigger* chosen = GetChosenTrigger();
	chosen->conditions.push_back(CCondition());
	CCondition* con = &chosen->conditions[ chosen->conditions.size()-1 ];
	strncpy(con->name, eb->value.c_str(), 15);
}

void Click_AddAction()
{	
	CView* v = g_GUI.getview("ed trigger");
	Widget* lb = v->getwidget("effects", LISTBOX);
	Widget* eb = v->getwidget("neweffect", EDITBOX);

	if(!UniqueEffect(eb->value.c_str()))
	{
		OpenAnotherView("name unique");
		return;
	}
	
	lb->options.push_back(eb->value);
	CTrigger* chosen = GetChosenTrigger();
	chosen->effects.push_back(CEffect());
	CEffect* eff = &chosen->effects[ chosen->effects.size()-1 ];
	strncpy(eff->name, eb->value.c_str(), 15);
}

void Click_RenCondition()
{
	CTrigger* e = GetChosenTrigger();

	if(e == NULL)
		return;
	
	CView* v = g_GUI.getview("ed trigger");
	Widget* lb = v->getwidget("conditions", LISTBOX);
	Widget* eb = v->getwidget("newcondition", EDITBOX);
	int sel = lb->selected;

	if(sel < 0)
		return;

	if(!UniqueCondition(eb->value.c_str()))
	{
		OpenAnotherView("name unique");
		return;
	}

	lb->options[sel] = eb->value;
	strncpy(e->conditions[sel].name, eb->value.c_str(), 15);
}

void Click_RenEffect()
{	
	CTrigger* t = GetChosenTrigger();

	if(t == NULL)
		return;
	
	CView* v = g_GUI.getview("ed trigger");
	Widget* lb = v->getwidget("effects", LISTBOX);

	if(lb == NULL)
	{
		MessageBox(g_hWnd, "lb null", "", NULL);
	}

	Widget* eb = v->getwidget("neweffect", EDITBOX);
	
	if(eb == NULL)
	{
		MessageBox(g_hWnd, "eb null", "", NULL);
	}

	int sel = lb->selected;

	if(sel < 0 || sel >= lb->options.size())
	{
		char msg[128];
		sprintf(msg, "sel = %d", sel);
		MessageBox(g_hWnd, msg, "", NULL);
		return;
	}

	if(!UniqueEffect(eb->value.c_str()))
	{
		OpenAnotherView("name unique");
		return;
	}

	lb->options[sel] = eb->value;
	strncpy(t->effects[sel].name, eb->value.c_str(), 15);
}

void Select_ConCaptB()
{
	if(g_selection.size() <= 0 || g_selectType != SELECT_BUILDING)
	{
		Chat("Invalid selection");
		g_selection.clear();
		g_selection2.clear();
	}
	else
	{
		CCondition* chosen = GetChosenCondition();
		chosen->target = g_selection[0];
		g_selection.clear();
		SelectionFunc = NULL;
		g_canselect = false;
		OpenAnotherView("ed condition");
		ReGUICondition();
	}
}

void GMessage(const char* text)
{
	Widget* gmessage = g_GUI.getview("game message")->getwidget("message", TEXTBOX);
	gmessage->text = text;
	g_lasthint.message = text;
	g_lasthint.graphic = "gui\\transp.png";
	Widget* graphic = g_GUI.getview("game message")->getwidget("graphic", IMAGE);
	graphic->tex = g_transparency;
	Click_LastHint();
}

void GMessageG(const char* tex, int w, int h)
{
	Widget* graphic = g_GUI.getview("game message")->getwidget("graphic", IMAGE);
	CreateTexture(graphic->tex, tex, true);
	graphic->pos[0] = g_width/2 - w/2;
	graphic->pos[2] = g_width/2 + w/2;
	graphic->pos[1] = g_height/2 + 150 - 44-44 - h;
	graphic->pos[3] = g_height/2 + 150 - 44-44;
	g_lasthint.graphic = tex;
	g_lasthint.gwidth = w;
	g_lasthint.gheight = h;
	//AddButton("gui\\buttonbg.png", "Continue", MAINFONT8, g_width/2 - 44, g_height/2 + 150 - 44 - 44, g_width/2 + 44, g_height/2 + 150 - 44, &Click_CloseMessage, NULL, NULL);
}

//return true if map switched
bool CTrigger::checkallmet()
{
	for(int i=0; i<conditions.size(); i++)
	{
		CCondition* c = &conditions[i];

		if(!c->met)
			return false;
	}

	return execute();
}

void CTrigger::resetconditions()
{
	for(int i=0; i<conditions.size(); i++)
	{
		CCondition* c = &conditions[i];
		c->met = false;
	}
}

//return true if map switched
bool CTrigger::execute()
{
	//g_log<<"execute effects "<<effects.size()<<endl;
	//g_log.flush();

	for(int i=0; i<effects.size(); i++)
	{
		CEffect* e = &effects[i];

		//g_log<<"textval: "<<e->textval.c_str()<<endl;
		//g_log.flush();

		switch(e->type)
		{
		case EFFECT_DISABTRIGGER:
			if(e->trigger != NULL)
				e->trigger->enabled = false;
			break;
		case EFFECT_ENABTRIGGER:
			if(e->trigger != NULL)
			{
				e->trigger->enabled = true;
				if(e->trigger->checkallmet())
					return true;
			}
			break;
		case EFFECT_EXECUTETRIGGER:
			if(e->trigger != NULL)
			{
				e->trigger->execute();
			}
			break;
		case EFFECT_MESSAGE:
			GMessage(e->textval.c_str());
			break;
		case EFFECT_MESSAGEIMG:
			GMessageG(e->textval.c_str(), e->imgdw, e->imgdh);
			break;
		case EFFECT_RESETCONDITIONS:
			if(e->trigger != NULL)
				e->trigger->resetconditions();
			break;
		case EFFECT_LOSS:
			FreeMap();
			g_mode = MENU;
			RedoGUI();
			return true;
		case EFFECT_VICTORY:
			LoadMap(e->textval.c_str());
			return true;
		default: break;
		}
	}

	return false;
}

//returns true if map switched
bool ConditionMet(int type, int target, float fval, int count, int owner)
{
	CTrigger* t = g_scripthead;

	vector<pTrigger> tocheck;

	while(t)
	{
		if(t->enabled)
		{
			for(int i=0; i<t->conditions.size(); i++)
			{
				CCondition* c = &t->conditions[i];

				if(c->met)
					continue;

				if(c->type != type)
					continue;

				switch(type)
				{
				case CONDITION_ONFINISHEDB:
					if(c->target == target)
						goto conditionmatch;
					break;
				case CONDITION_ONPLACEB:
					if(c->target == target)
						goto conditionmatch;
					break;
				case CONDITION_ONBUILDU:
					if(c->target == target)
						goto conditionmatch;
					break;
				case CONDITION_ONPLACEOTHERB:
					if(c->target != target)
						goto conditionmatch;
					break;
				case CONDITION_CAPTUREB:
					if(c->target == target)
						goto conditionmatch;
					break;
				case CONDITION_MESSAGECLOSED:
				case CONDITION_ONLOADMAP:
				case CONDITION_ALLCONSTRUCTIONFINI:
				case CONDITION_ALLROADACCESSIBLE:
				case CONDITION_ROADINACCESSABILITY:
					goto conditionmatch;
					break;
				case CONDITION_PRICESET:
					if(c->target == target && c->fval == fval)
						goto conditionmatch;
					break;
				default: break;
				}

				continue;

conditionmatch:
				c->met = true;
					
				// execute only after all triggers' conditions have been checked
				tocheck.push_back(t);
			}
		}

		t = t->next;
	}

	for(int i=0; i<tocheck.size(); i++)
	{
		if(tocheck[i]->checkallmet())	//returns true if map switched
			return true;
	}

	return false;
}

void OnLoadMap()
{
	ConditionMet(CONDITION_ONLOADMAP, 0, 0, 0, 0);
}

void OnRoadInacc()
{
	ConditionMet(CONDITION_ROADINACCESSABILITY, 0, 0, 0, 0);
}

void OnAllRoadAc()
{
	ConditionMet(CONDITION_ALLROADACCESSIBLE, 0, 0, 0, 0);
}

void OnCaptureB(int target)
{
	ConditionMet(CONDITION_CAPTUREB, target, 0, 0, 0);
}

void OnPlaceB(int target)
{	
	if(ConditionMet(CONDITION_ONPLACEB, target, 0, 0, 0))
		return;
	if(ConditionMet(CONDITION_ONPLACEOTHERB, target, 0, 0, 0))
		return;
}

void OnBuildU(int target)
{
	g_log<<"on buid u "<<target<<endl;
	ConditionMet(CONDITION_ONBUILDU, target, 0, 0, 0);
}

void OnFinishedB(int target)
{
	if(ConditionMet(CONDITION_ONFINISHEDB, target, 0, 0, 0))
		return;
	
	//check if all the player's constructions are finished
	
	for(int i=0; i<BUILDINGS; i++)
	{
		CBuilding* b = &g_building[i];

		if(b->on && !b->finished && b->owner == g_localP)
			return;
	}

	for(int x=0; x<g_hmap.m_widthX; x++)
		for(int z=0; z<g_hmap.m_widthZ; z++)
		{
			if(RoadAt(x, z)->on && !RoadAt(x, z)->finished && RoadAt(x, z)->owner == g_localP)
				return;
			if(PowlAt(x, z)->on && !PowlAt(x, z)->finished && PowlAt(x, z)->owner == g_localP)
				return;
			if(PipeAt(x, z)->on && !PipeAt(x, z)->finished && PipeAt(x, z)->owner == g_localP)
				return;
		}

	ConditionMet(CONDITION_ALLCONSTRUCTIONFINI, 0, 0, 0, 0);
}

void OnPriceSet(int targ, float val)
{
	ConditionMet(CONDITION_PRICESET, targ, val, 0, 0);
}

void OnCloseMessage()
{
	ConditionMet(CONDITION_MESSAGECLOSED, 0, 0, 0, 0);
}

void Click_CloseMessage()
{
	CloseView("game message");
	OnCloseMessage();
}

void ReGUICondition()
{
	CCondition* chosen = GetChosenCondition();

	if(chosen == NULL)
		return;

	CView* v = g_GUI.getview("ed condition");
	Widget* dd = v->getwidget("type", DROPDOWN);
	dd->select(chosen->type);

	CView* v2;
	Widget* dd2;
	Widget* w;
	char buffer[128];
	CBuilding* bldg;

	switch(chosen->type)
	{
	case CONDITION_NONE:	break;
	case CONDITION_ONLOADMAP:	break;
	case CONDITION_ONPLACEB:
	case CONDITION_ONPLACEOTHERB:
	case CONDITION_ONFINISHEDB:
		OpenAnotherView("ed con selb");
		v2 = g_GUI.getview("ed con selb");
		dd2 = v2->getwidget("build", DROPDOWN);
		dd2->select(chosen->target);
		break;
	case CONDITION_ONBUILDU:
	case CONDITION_ONBUILDOTHERU:
		OpenAnotherView("ed con selu");
		v2 = g_GUI.getview("ed con selu");
		dd2 = v2->getwidget("unitt", DROPDOWN);
		dd2->select(chosen->target);
		break;
	case CONDITION_PRICESET:
		OpenAnotherView("ed con selr");
		OpenAnotherView("ed con price");
		v2 = g_GUI.getview("ed con selr");
		dd2 = v2->getwidget("res", DROPDOWN);
		dd2->select(chosen->target);
		v2 = g_GUI.getview("ed con price");
		w = v2->getwidget("price", EDITBOX);
		sprintf(buffer, "%f", chosen->fval);
		w->value = buffer;
		break;
	case CONDITION_UNITCOUNT:
		OpenAnotherView("ed con ucount");
		v2 = g_GUI.getview("ed con ucount");
		w = v2->getwidget("owner", DROPDOWN);
		w->select(chosen->owner);
		sprintf(buffer, "%d", chosen->count);
		w = v2->getwidget("count", EDITBOX);
		w->value = buffer;
		w = v2->getwidget("unitt", DROPDOWN);
		w->select(chosen->target);
		break;
	case CONDITION_CAPTUREB:
		OpenAnotherView("ed con mapb");
		v2 = g_GUI.getview("ed con mapb");
		if(chosen->target < 0)
		{
			w = v2->getwidget("owner", TEXT);
			w->text = "";
			w = v2->getwidget("building", TEXT);
			w->text = "";
		}
		else
		{
			bldg = &g_building[chosen->target];
			w = v2->getwidget("owner", TEXT);
			w->text = facnamestr[bldg->owner];
			w = v2->getwidget("building", TEXT);
			w->text = g_buildingType[bldg->type].name;
		}
		break;
	default: break;
	}
}

void Click_EdCondition()
{
	if(GetChosenCondition() == NULL)
		return;

	ReGUICondition();
	CloseView("ed trigger");
	OpenAnotherView("ed condition");
}

void Change_ConUCntType()
{
	CCondition* chosen = GetChosenCondition();

	if(chosen == NULL)
		return;

	CView* v = g_GUI.getview("ed con ucount");
	Widget* owner = v->getwidget("unitt", DROPDOWN);
	chosen->target = owner->selected;
}

void Change_ConUCntOwner()
{
	CCondition* chosen = GetChosenCondition();

	if(chosen == NULL)
		return;

	CView* v = g_GUI.getview("ed con ucount");
	Widget* owner = v->getwidget("owner", DROPDOWN);
	chosen->owner = owner->selected;
}

void Change_ConUCntCount(int dummy)
{
	CCondition* chosen = GetChosenCondition();

	if(chosen == NULL)
		return;

	CView* v = g_GUI.getview("ed con ucount");
	Widget* count = v->getwidget("count", EDITBOX);
	chosen->count = strToInt(count->value.c_str());
}

void ReGUIEffect()
{
	CEffect* chosen = GetChosenEffect();

	if(chosen == NULL)
		return;
	
	CloseEffect();
	OpenAnotherView("ed effect");

	CView* v = g_GUI.getview("ed effect");
	Widget* dd = v->getwidget("type", DROPDOWN);
	dd->select(chosen->type);
	
	CView* v2;
	Widget* dd2;
	CTrigger* trigger = g_scripthead;
	Widget* textarea;
	Widget* editbox;
	Widget* graphic;
	int centerx = g_width/2;
	int centery = g_height/2+50;
	char wstr[16];
	char hstr[16];

	switch(chosen->type)
	{
	case EFFECT_NONE:	break;
	case EFFECT_ENABTRIGGER:
	case EFFECT_DISABTRIGGER:
	case EFFECT_RESETCONDITIONS:
	case EFFECT_EXECUTETRIGGER:
		OpenAnotherView("ed eff selt");
		v2 = g_GUI.getview("ed eff selt");
		dd2 = v2->getwidget("trigger", DROPDOWN);
		dd2->clear();
		while(trigger)
		{
			dd2->options.push_back(trigger->name);
			if(chosen->trigger == trigger)
				dd2->selected = dd2->options.size()-1;
			trigger = trigger->next;
		}
		break;
	case EFFECT_MESSAGE:
		OpenAnotherView("ed eff message");
		textarea = g_GUI.getview("ed eff message")->getwidget("message", TEXTAREA);
		textarea->changevalue(chosen->textval.c_str());
		break;
	case EFFECT_MESSAGEIMG:
		OpenAnotherView("ed eff selg");
		editbox = g_GUI.getview("ed eff selg")->getwidget("path", EDITBOX);
		editbox->changevalue(chosen->textval.c_str());
		graphic = g_GUI.getview("ed eff selg")->getwidget("graphic", IMAGE);
		CreateTexture(graphic->tex, chosen->textval.c_str(), true);
		graphic->pos[0] = centerx - chosen->imgdw/2;
		graphic->pos[1] = centery - chosen->imgdh/2;
		graphic->pos[2] = centerx + chosen->imgdw/2;
		graphic->pos[3] = centery + chosen->imgdh/2;
		sprintf(wstr, "%d", chosen->imgdw);
		sprintf(hstr, "%d", chosen->imgdh);
		editbox = g_GUI.getview("ed eff selg")->getwidget("width", EDITBOX);
		editbox->value = wstr;
		editbox = g_GUI.getview("ed eff selg")->getwidget("height", EDITBOX);
		editbox->value = hstr;
		break;
	case EFFECT_VICTORY:
		OpenAnotherView("ed eff selm");
		editbox = g_GUI.getview("ed eff selm")->getwidget("path", EDITBOX);
		editbox->changevalue(chosen->textval.c_str());
		break;
	default: break;
	}
}

void Click_EdEffect()
{
	if(GetChosenEffect() == NULL)
		return;

	ReGUIEffect();
	CloseView("ed trigger");
	OpenAnotherView("ed effect");
}

void Click_DelCondition()
{
	CTrigger* e = GetChosenTrigger();

	if(e == NULL)
		return;
	
	CView* v = g_GUI.getview("ed trigger");
	Widget* lb = v->getwidget("conditions", LISTBOX);
	int sel = lb->selected;

	if(sel < 0)
		return;

	lb->erase(sel);
	e->conditions.erase( e->conditions.begin() + sel );
}

void Click_DelEffect()
{
	CTrigger* t = GetChosenTrigger();

	if(t == NULL)
		return;
	
	CView* v = g_GUI.getview("ed trigger");
	Widget* lb = v->getwidget("effects", LISTBOX);
	int sel = lb->selected;
	
	if(sel < 0)
		return;

	lb->erase(sel);
	t->effects.erase( t->effects.begin() + sel );
}

void Click_RenameTrigger()
{
	CTrigger* s = GetChosenTrigger();
	if(s == NULL)
		return;
	
	CView* v = g_GUI.getview("ed script");
	Widget* lb = v->getwidget("triggers", LISTBOX);
	Widget* eb = v->getwidget("newtrigger", EDITBOX);
	if(!UniqueTrigger(eb->value.c_str()))
	{
		OpenAnotherView("name unique");
		return;
	}

	lb->options[lb->selected] = eb->value;
	strncpy(s->name, eb->value.c_str(), 15);
}

void Change_NewTrigger(int dummy)
{
}

CTrigger* GetTrigger(int which)
{
	if(which < 0)
		return NULL;

	CTrigger* curs = g_scripthead;
	int curi = 0;

	while(curs)
	{
		if(curi == which)
			return curs;

		curi ++;
		curs = curs->next;
	}

	return NULL;
}

int TriggerID(CTrigger* trigger)
{
	int cur = 0;
	CTrigger* curt = g_scripthead;

	while(curt)
	{
		if(curt == trigger)
			return cur;

		curt = curt->next;
		cur++;
	}

	return -1;
}

CTrigger* GetChosenTrigger()
{
	CView* edscript = g_GUI.getview("ed script");
	Widget* triggers = edscript->getwidget("triggers", LISTBOX);
	int selected = triggers->selected;
	return GetTrigger(selected);
}

CCondition* GetChosenCondition()
{
	CView* edscript = g_GUI.getview("ed script");
	Widget* triggers = edscript->getwidget("triggers", LISTBOX);
	int selected = triggers->selected;
	CTrigger* t = GetTrigger(selected);

	if(t == NULL)
		return NULL;

	CView* edtrigger = g_GUI.getview("ed trigger");
	Widget* conditions = edtrigger->getwidget("conditions", LISTBOX);
	selected = conditions->selected;

	if(selected < 0)
		return NULL;

	if(selected >= t->conditions.size())
		return NULL;

	return &t->conditions[selected];
}

CEffect* GetChosenEffect()
{
	CView* edscript = g_GUI.getview("ed script");
	Widget* triggers = edscript->getwidget("triggers", LISTBOX);
	int selected = triggers->selected;
	CTrigger* t = GetTrigger(selected);

	if(t == NULL)
		return NULL;

	CView* edtrigger = g_GUI.getview("ed trigger");
	Widget* effects = edtrigger->getwidget("effects", LISTBOX);
	selected = effects->selected;

	if(selected < 0)
		return NULL;

	if(selected >= t->effects.size())
		return NULL;

	return &t->effects[selected];
}

CTrigger* GetScriptTail()
{
	if(g_scripthead == NULL)
		return NULL;

	CTrigger* thise = g_scripthead;

	while(true)
	{
		if(thise->next != NULL)
			thise = thise->next;
		else
			return thise;
	}

	return NULL;
}

void ReGUIScript()
{
	CView* edscript = g_GUI.getview("ed script");
	Widget* events = edscript->getwidget("triggers", LISTBOX);
	events->clear();

	CTrigger* cure = g_scripthead;
	while(cure)
	{
		events->options.push_back(cure->name);
		cure = cure->next;
	}
}

void ReGUITrigger()
{
	CTrigger* t = GetChosenTrigger();

	if(t == NULL)
		return;

	CView* edtrigger = g_GUI.getview("ed trigger");
	Widget* conditions = edtrigger->getwidget("conditions", LISTBOX);
	Widget* effects = edtrigger->getwidget("effects", LISTBOX);
	conditions->clear();
	effects->clear();

	for(int i=0; i<t->conditions.size(); i++)
	{
		conditions->options.push_back(t->conditions[i].name);
	}

	for(int i=0; i<t->effects.size(); i++)
	{
		effects->options.push_back(t->effects[i].name);
	}

	Widget* enabled = edtrigger->getwidget("enabled", CHECKBOX);
	enabled->selected = t->enabled ? 1 : 0;
}

void FreeScript()
{
	CTrigger* thise = GetScriptTail();

	if(thise == NULL)
		return;

	while(true)
	{
		if(thise->prev != NULL)
		{
			thise = thise->prev;
			delete thise->next;
			thise->next = NULL;
		}
		else
		{
			delete thise;
			break;
		}
	}

	g_scripthead = NULL;
}

bool UniqueTrigger(const char* name)
{
	CTrigger* cure = g_scripthead;

	while(cure)
	{
		if(stricmp(cure->name, name) == 0)
			return false;

		cure = cure->next;
	}

	return true;
}

bool UniqueEffect(const char* name)
{
	CTrigger* t = GetChosenTrigger();

	if(t == NULL)
		return false;

	for(int i=0; i<t->effects.size(); i++)
	{
		if(stricmp(t->effects[i].name, name) == 0)
			return false;
	}

	return true;
}

bool UniqueCondition(const char* name)
{
	CTrigger* e = GetChosenTrigger();

	if(e == NULL)
		return false;

	for(int i=0; i<e->conditions.size(); i++)
	{
		if(stricmp(e->conditions[i].name, name) == 0)
			return false;
	}

	return true;
}

void Change_ConRes()
{
	CCondition* chosen = GetChosenCondition();

	if(chosen == NULL)
		return;

	CView* v = g_GUI.getview("ed con selr");
	Widget* resdropdown = v->getwidget("res", DROPDOWN);
	chosen->target = resdropdown->selected;
}

void Click_ConSelB()
{
	CloseCondition();
	SelectionFunc = &Select_ConCaptB;
	g_canselect = true;
}

void Change_ConPrice(int dummy)
{
	CCondition* chosen = GetChosenCondition();

	if(chosen == NULL)
		return;

	CView* v = g_GUI.getview("ed con price");
	Widget* pricebox = v->getwidget("price", EDITBOX);
	chosen->fval = strToFloat(pricebox->value.c_str());
}

void Change_ConUnit()
{
	CCondition* chosen = GetChosenCondition();

	if(chosen == NULL)
		return;

	CView* v = g_GUI.getview("ed con selu");
	Widget* dd = v->getwidget("unitt", DROPDOWN);
	chosen->target = dd->selected;
}

void Change_EffTrigger()
{
	CView* v = g_GUI.getview("ed eff selt");
	Widget* dd = v->getwidget("trigger", DROPDOWN);

	CEffect* eff = GetChosenEffect();

	if(eff == NULL)
		return;

	CTrigger* trig = GetTrigger(dd->selected);

	if(trig == NULL)
		return;

	eff->trigger = trig;
}

void Change_ConBuild()
{
	CCondition* chosen = GetChosenCondition();

	if(chosen == NULL)
		return;

	CView* v = g_GUI.getview("ed con selb");
	Widget* dd = v->getwidget("build", DROPDOWN);
	chosen->target = dd->selected;
}

void Change_CondType()
{
	CCondition* chosen = GetChosenCondition();

	if(chosen == NULL)
		return;

	CloseCondition();
	OpenAnotherView("ed condition");

	CView* v = g_GUI.getview("ed condition");
	Widget* dd = v->getwidget("type", DROPDOWN);
	chosen->type = dd->selected;
	
	if(chosen->type == CONDITION_CAPTUREB)
		chosen->target = -1;

	ReGUICondition();
}

void Change_EffType()
{
	CEffect* chosen = GetChosenEffect();

	if(chosen == NULL)
		return;

	CView* v = g_GUI.getview("ed effect");
	Widget* dd = v->getwidget("type", DROPDOWN);
	chosen->type = dd->selected;

	ReGUIEffect();
}

void CloseEffect()
{
	CloseView("ed effect");
	CloseView("ed eff selt");
	CloseView("ed eff message");
	CloseView("ed eff selg");
	CloseView("ed eff selm");
}

void CloseCondition()
{
	CloseView("ed condition");
	CloseView("ed con selb");
	CloseView("ed con selu");
	CloseView("ed con selr");
	CloseView("ed con price");
	CloseView("ed con ucount");
	CloseView("ed con mapb");
}

void Change_Triggers()
{
	CloseView("ed trigger");
	CloseEffect();
}

void Click_CloseCondition()
{
	CloseCondition();
	OpenAnotherView("ed trigger");
}

void Click_CloseEffect()
{
	CloseEffect();
	OpenAnotherView("ed trigger");
}

void Click_CloseTrigger()
{
	CloseView("ed trigger");
	//CloseView("ed condition");
	//CloseView("ed con selb");
	//CloseView("ed con selu");
}

void Click_MessageR(int res)
{
	Widget* textarea = g_GUI.getview("ed eff message")->getwidget("message", TEXTAREA);
	textarea->opened = true;
	if(res < RESOURCES)
	{
		CResource* r = &g_resource[res];
		textarea->charin((int)r->icon);
	}
	else if(res == RESOURCES)
	{
		textarea->charin((int)TIME_CHAR);
	}
}

void Change_EffGPath(int dummy)
{
	Widget* editbox = g_GUI.getview("ed eff selg")->getwidget("path", EDITBOX);
	CEffect* chosen = GetChosenEffect();

	if(chosen == NULL)
		return;

	chosen->textval = editbox->value;

	Widget* graphic = g_GUI.getview("ed eff selg")->getwidget("graphic", IMAGE);
	CreateTexture(graphic->tex, chosen->textval.c_str(), true);
}

void Change_EffMPath(int dummy)
{
	Widget* editbox = g_GUI.getview("ed eff selm")->getwidget("path", EDITBOX);
	CEffect* chosen = GetChosenEffect();

	if(chosen == NULL)
		return;

	chosen->textval = editbox->value;
}

void Change_EffGSize(int dummy)
{
	CEffect* chosen = GetChosenEffect();
	
	if(!chosen)
		return;

	CView* v = g_GUI.getview("ed eff selg");
	Widget* ww = v->getwidget("width", EDITBOX);
	Widget* wh = v->getwidget("height", EDITBOX);
	int width = strToInt(ww->value.c_str());
	int height = strToInt(wh->value.c_str());
	Widget* graphic = v->getwidget("graphic", IMAGE);
	int centerx = g_width/2;
	int centery = g_height/2+50;
	graphic->pos[0] = centerx - width/2;
	graphic->pos[1] = centery - height/2;
	graphic->pos[2] = centerx + width/2;
	graphic->pos[3] = centery + height/2;
	chosen->imgdw = width;
	chosen->imgdh = height;
}

void Click_EffPickMPath()
{
	OPENFILENAME ofn;
	
	char filepath[MAX_PATH+1];
	
	ZeroMemory( &ofn , sizeof( ofn));

	ofn.lStructSize = sizeof ( ofn );
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = filepath;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof( filepath );
	//ofn.lpstrFilter = "Save\0*.map\0All\0*.*\0";
	ofn.lpstrFilter = "All\0*.*\0";
	ofn.nFilterIndex =1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = MAX_PATH;	//0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST;

	if(!GetOpenFileName(&ofn))
		return;

	CEffect* chosen = GetChosenEffect();

	if(chosen == NULL)
		return;
	
	Widget* editbox = g_GUI.getview("ed eff selm")->getwidget("path", EDITBOX);
	editbox->value = MakePathRelative(filepath);
	Change_EffMPath(0);
}

void Click_EffPickGPath()
{
	OPENFILENAME ofn;
	
	char filepath[MAX_PATH+1];
	
	ZeroMemory( &ofn , sizeof( ofn));

	ofn.lStructSize = sizeof ( ofn );
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = filepath;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof( filepath );
	//ofn.lpstrFilter = "Save\0*.map\0All\0*.*\0";
	ofn.lpstrFilter = "All\0*.*\0";
	ofn.nFilterIndex =1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = MAX_PATH;	//0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST;

	if(!GetOpenFileName(&ofn))
		return;

	CEffect* chosen = GetChosenEffect();

	if(chosen == NULL)
		return;
	
	Widget* editbox = g_GUI.getview("ed eff selg")->getwidget("path", EDITBOX);
	editbox->value = MakePathRelative(filepath);
	Change_EffGPath(0);
}

void Change_Message()
{
	Widget* textarea = g_GUI.getview("ed eff message")->getwidget("message", TEXTAREA);
	CEffect* chosen = GetChosenEffect();

	if(chosen == NULL)
		return;

	chosen->textval = textarea->value;
}

void Click_CloseUnique()
{
	CloseView("name unique");
}

void Change_TrigEnabled()
{
	CTrigger* t = GetChosenTrigger();

	if(t == NULL)
		return;

	CView* edtrigger = g_GUI.getview("ed trigger");
	Widget* enabled = edtrigger->getwidget("enabled", CHECKBOX);
	t->enabled = enabled->selected <= 0 ? false : true;
}