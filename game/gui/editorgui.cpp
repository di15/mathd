

#include "../gmain.h"
#include "../../common/gui/gui.h"
#include "../keymap.h"
#include "../../common/render/heightmap.h"
#include "../../common/math/camera.h"
#include "../../common/render/shadow.h"
#include "../../common/render/screenshot.h"
#include "../../common/save/savemap.h"
#include "ggui.h"
#include "../../common/sim/unittype.h"
#include "editorgui.h"
#include "../../common/save/savemap.h"
#include "../../common/sim/buildingtype.h"
#include "../../common/sim/country.h"

void Resize_LeftPanel(Widget* thisw)
{
	thisw->m_pos[0] = 0;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = LEFT_PANEL_WIDTH;
	thisw->m_pos[3] = g_height;
}

void Resize_EditorToolsSelector(Widget* thisw)
{
	thisw->m_pos[0] = 5;
	thisw->m_pos[1] = 5 + 64 + SCROLL_BORDER;
	thisw->m_pos[2] = LEFT_PANEL_WIDTH - 5;
	thisw->m_pos[3] = 5 + 64 + SCROLL_BORDER + g_font[thisw->m_font].gheight + 5;
}

int EdToolsSelection()
{
	View* editorguiview = g_GUI.getview("editor gui");

	Widget* edtoolsselector = editorguiview->getwidget("editor tools selector", WIDGET_DROPDOWNSELECTOR);

	int selected = edtoolsselector->m_selected;

	return selected;
}

void CloseEdTools()
{
	CloseView("borders");
	CloseView("place units");
	CloseView("place buildings");
	CloseView("place roads");
	CloseView("place powerlines");
	CloseView("place crude pipelines");

	g_build = BUILDING_NONE;
}

int GetEdTool()
{
	View* editorguiview = g_GUI.getview("editor gui");

	Widget* edtoolsselector = editorguiview->getwidget("editor tools selector", WIDGET_DROPDOWNSELECTOR);

	int selected = edtoolsselector->m_selected;

	return selected;
}

void Change_EditorToolsSelector()
{
	int selected = GetEdTool();

	CloseEdTools();

	// Borders
	if(selected == EDTOOL_BORDERS)
	{
		OpenAnotherView("borders");
	}
	// Place Units
	else if(selected == EDTOOL_PLACEUNITS)	
	{
		OpenAnotherView("place units");
	}
	// Place Buildings
	else if(selected == EDTOOL_PLACEBUILDINGS)
	{
		OpenAnotherView("place buildings");
	}
	// Delete Objects
	else if(selected == EDTOOL_DELETEOBJECTS)
	{
		OpenAnotherView("delete objects");
	}
	else if(selected == EDTOOL_PLACEROADS)
	{
		g_build = BUILDING_ROAD;
	}
	else if(selected == EDTOOL_PLACECRUDEPIPES)
	{
		g_build = BUILDING_CRUDEPIPE;
	}
	else if(selected == EDTOOL_PLACEPOWERLINES)
	{
		g_build = BUILDING_POWERL;
	}
#if 0
#define EDTOOL_PLACEROADS		4
#define EDTOOL_PLACEPOWERLINES	5
#define EDTOOL_PLACECRUDEPIPES	6
#endif
}

void Resize_UnitDropDownSelector(Widget* thisw)
{
	thisw->m_pos[0] = 5;
	thisw->m_pos[1] = 5 + 64 + SCROLL_BORDER + 20;
	thisw->m_pos[2] = LEFT_PANEL_WIDTH - 5;
	thisw->m_pos[3] = thisw->m_pos[1] + g_font[thisw->m_font].gheight + 5;
}

void Resize_UnitCountryDropDownSelector(Widget* thisw)
{
	thisw->m_pos[0] = 5;
	thisw->m_pos[1] = 5 + 64 + SCROLL_BORDER + 20*3;
	thisw->m_pos[2] = LEFT_PANEL_WIDTH - 5;
	thisw->m_pos[3] = thisw->m_pos[1] + g_font[thisw->m_font].gheight + 5;
}

void Resize_UnitCompanyDropDownSelector(Widget* thisw)
{
	thisw->m_pos[0] = 5;
	thisw->m_pos[1] = 5 + 64 + SCROLL_BORDER + 20*4;
	thisw->m_pos[2] = LEFT_PANEL_WIDTH - 5;
	thisw->m_pos[3] = thisw->m_pos[1] + g_font[thisw->m_font].gheight + 5;
}

void Resize_BuildingDropDownSelector(Widget* thisw)
{
	thisw->m_pos[0] = 5;
	thisw->m_pos[1] = 5 + 64 + SCROLL_BORDER + 20;
	thisw->m_pos[2] = LEFT_PANEL_WIDTH - 5;
	thisw->m_pos[3] = thisw->m_pos[1] + g_font[thisw->m_font].gheight + 5;
}

void Change_UnitDropDownSelector()
{
}

void FillPlaceUnitsView()
{
	View* placeunitsview = AddView("place units");
	
	placeunitsview->widget.push_back(new DropDownS(NULL, "unit selector", MAINFONT8, Resize_UnitDropDownSelector, NULL));

	Widget* unitselector = placeunitsview->getwidget("unit selector", WIDGET_DROPDOWNSELECTOR);
	
	for(int i=0; i<UNIT_TYPES; i++)
		unitselector->m_options.push_back(RichText(g_unitT[i].name));

	//unitselector->m_options.push_back(RichText("Labourer"));
	//unitselector->m_options.push_back(RichText("Truck"));
	//unitselector->m_options.push_back(RichText("Infantry"));

	//unitselector->select(0);
	
	placeunitsview->widget.push_back(new DropDownS(NULL, "unit country selector", MAINFONT8, Resize_UnitCountryDropDownSelector, NULL));

	Widget* unitcountryselector = placeunitsview->getwidget("unit country selector", WIDGET_DROPDOWNSELECTOR);
	
	unitcountryselector->m_options.push_back(RichText("No country"));
	for(int i=0; i<COUNTRIES; i++)
		unitcountryselector->m_options.push_back(g_country[i].name);

	placeunitsview->widget.push_back(new DropDownS(NULL, "unit company selector", MAINFONT8, Resize_UnitCompanyDropDownSelector, NULL));

	Widget* unitcompanyselector = placeunitsview->getwidget("unit company selector", WIDGET_DROPDOWNSELECTOR);

	unitcompanyselector->m_options.push_back(RichText("No cmpany"));
}

int GetPlaceUnitCountry()
{
	View* placeunitsview = g_GUI.getview("place units");

	Widget* unitcountryselector = placeunitsview->getwidget("unit country selector", WIDGET_DROPDOWNSELECTOR);

	return unitcountryselector->m_selected - 1;
}

int GetPlaceUnitCompany()
{
	View* placeunitsview = g_GUI.getview("place units");
	
	Widget* unitcompanyselector = placeunitsview->getwidget("unit company selector", WIDGET_DROPDOWNSELECTOR);

	return unitcompanyselector->m_selected - 1;
}

void FillPlaceBuildingsView()
{
	View* placebuildingsview = AddView("place buildings");

	placebuildingsview->widget.push_back(new DropDownS(NULL, "building selector", MAINFONT8, Resize_BuildingDropDownSelector, NULL));

	Widget* buildingselector = placebuildingsview->getwidget("building selector", WIDGET_DROPDOWNSELECTOR);
	
	for(int i=0; i<BUILDING_TYPES; i++)
	{
		buildingselector->m_options.push_back(RichText(g_buildingT[i].name));
	}
	
	//placebuildingsview->widget.push_back(new DropDownS(NULL, "country owner selector", MAINFONT8, Resize_BuildingCountryDropDownSelector, NULL));

	//placebuildingsview->widget.push_back(new DropDownS(NULL, "company owner selector", MAINFONT8, Resize_BuildingCompanyDropDownSelector, NULL));
}

int GetPlaceUnitType()
{
	View* placeunitsview = g_GUI.getview("place units");
	
	Widget* unitselector = placeunitsview->getwidget("unit selector", WIDGET_DROPDOWNSELECTOR);

	return unitselector->m_selected;
}

int GetPlaceBuildingType()
{
	View* placebuildingsview = g_GUI.getview("place buildings");
	
	Widget* buildingselector = placebuildingsview->getwidget("building selector", WIDGET_DROPDOWNSELECTOR);

	return buildingselector->m_selected;
}

void Resize_LoadMapButton(Widget* thisw)
{
	thisw->m_pos[0] = 5 + 64*0;
	thisw->m_pos[1] = 5 + SCROLL_BORDER;
	thisw->m_pos[2] = 5 + 64*1;
	thisw->m_pos[3] = thisw->m_pos[1] + 64;
}

void Resize_SaveMapButton(Widget* thisw)
{
	thisw->m_pos[0] = 5 + 64*1;
	thisw->m_pos[1] = 5 + SCROLL_BORDER;
	thisw->m_pos[2] = 5 + 64*2;
	thisw->m_pos[3] = thisw->m_pos[1] + 64;
}

void Resize_QSaveMapButton(Widget* thisw)
{
	thisw->m_pos[0] = 5 + 64*2;
	thisw->m_pos[1] = 5 + SCROLL_BORDER;
	thisw->m_pos[2] = 5 + 64*3;
	thisw->m_pos[3] = thisw->m_pos[1] + 64;
}

void FillEditorGUI()
{
	g_lastsave[0] = '\0';

	View* editorguiview = AddView("editor gui");

	editorguiview->widget.push_back(new Image(NULL, "gui/filled.jpg", Resize_LeftPanel));
	
	editorguiview->widget.push_back(new Button(NULL, "gui/edload.png", RichText("Load map"), MAINFONT8, Resize_LoadMapButton, Click_LoadMapButton, NULL, NULL));
	editorguiview->widget.push_back(new Button(NULL, "gui/edsave.png", RichText("Save map"), MAINFONT8, Resize_SaveMapButton, Click_SaveMapButton, NULL, NULL));
	editorguiview->widget.push_back(new Button(NULL, "gui/qsave.png", RichText("Quick save"), MAINFONT8, Resize_QSaveMapButton, Click_QSaveMapButton, NULL, NULL));

	editorguiview->widget.push_back(new DropDownS(NULL, "editor tools selector", MAINFONT8, Resize_EditorToolsSelector, Change_EditorToolsSelector));

	Widget* edtoolsselector = editorguiview->getwidget("editor tools selector", WIDGET_DROPDOWNSELECTOR);
	
#if 0
#define EDTOOL_BORDERS			0
#define EDTOOL_PLACEUNITS		1
#define EDTOOL_PLACEBUILDINGS	2
#define EDTOOL_DELETEOBJECTS	3
#endif 

	edtoolsselector->m_options.push_back(RichText("Borders"));
	edtoolsselector->m_options.push_back(RichText("Place Units"));
	edtoolsselector->m_options.push_back(RichText("Place Buildings"));
	edtoolsselector->m_options.push_back(RichText("Delete Objects"));
	edtoolsselector->m_options.push_back(RichText("Place Roads"));
	edtoolsselector->m_options.push_back(RichText("Place Powerline"));
	edtoolsselector->m_options.push_back(RichText("Place Crude Oil Pipeline"));

	//edtoolsselector->select(1);

	FillPlaceUnitsView();
	FillPlaceBuildingsView();
}
