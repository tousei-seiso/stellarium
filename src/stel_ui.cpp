/*
 * Stellarium
 * Copyright (C) 2002 Fabien Ch�reau
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

// Class which handles a stellarium User Interface

#include "stel_ui.h"
#include "stellastro.h"
#include <iostream>

////////////////////////////////////////////////////////////////////////////////
//								CLASS FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

stel_ui::stel_ui(stel_core * _core) :
	spaceFont(NULL),
	courierFont(NULL),

	top_bar_ctr(NULL),
	top_bar_date_lbl(NULL),
	top_bar_hour_lbl(NULL),
	top_bar_fps_lbl(NULL),
	top_bar_appName_lbl(NULL),
	top_bar_fov_lbl(NULL),

	bt_flag_ctr(NULL),
	bt_flag_constellation_draw(NULL),
	bt_flag_constellation_name(NULL),
	bt_flag_azimuth_grid(NULL),
	bt_flag_equator_grid(NULL),
	bt_flag_ground(NULL),
	bt_flag_cardinals(NULL),
	bt_flag_atmosphere(NULL),
	bt_flag_nebula_name(NULL),
	bt_flag_help(NULL),
	bt_flag_equatorial_mode(NULL),
	bt_flag_config(NULL),
	bt_flag_help_lbl(NULL),

	info_select_ctr(NULL),
	info_select_txtlbl(NULL),

	licence_win(NULL),
	licence_txtlbl(NULL),

	help_win(NULL),
	help_txtlbl(NULL),

	config_win(NULL)
{
	if (!_core)
	{
		printf("ERROR : In stel_ui constructor, unvalid core.");
		exit(-1);
	}
	core = _core;
}

////////////////////////////////////////////////////////////////////////////////
void stel_ui::init(void)
{
    // Load standard font
    spaceFont = new s_font(18, "spacefont", core->DataDir + "spacefont.txt");
    if (!spaceFont)
    {
        printf("ERROR WHILE CREATING FONT\n");
        exit(-1);
    }

	courierFont = new s_font(12.5, "courierfont", core->DataDir + "courierfont.txt");
    if (!courierFont)
    {
        printf("ERROR WHILE CREATING FONT\n");
        exit(-1);
    }

	// Create standard texture
	baseTex = new s_texture("backmenu", TEX_LOAD_TYPE_PNG_ALPHA);
	flipBaseTex = new s_texture("backmenu_flip", TEX_LOAD_TYPE_PNG_ALPHA);

	tex_up = new s_texture("up");
	tex_down = new s_texture("down");

	// Set default Painter
	Painter p(baseTex, spaceFont, core->GuiBaseColor, core->GuiTextColor);
	Component::setDefaultPainter(p);

	Component::initScissor(core->screen_W, core->screen_H);

	desktop = new Container();
	desktop->reshape(0,0,core->screen_W,core->screen_H);

	bt_flag_help_lbl = new Label("ERROR...");
	bt_flag_help_lbl->setPos(3,core->screen_H-40);
	bt_flag_help_lbl->setVisible(0);

	// Info on selected object
	info_select_ctr = new Container();
	info_select_ctr->reshape(0,15,300,80);
    info_select_txtlbl = new TextLabel("Info");
    info_select_txtlbl->reshape(5,5,290,78);
    info_select_ctr->setVisible(0);
	info_select_ctr->addComponent(info_select_txtlbl);
	desktop->addComponent(info_select_ctr);

	desktop->addComponent(createTopBar());
	desktop->addComponent(createFlagButtons());
	desktop->addComponent(bt_flag_help_lbl);
	desktop->addComponent(createLicenceWindow());
	desktop->addComponent(createHelpWindow());
	desktop->addComponent(createConfigWindow());

}

////////////////////////////////////////////////////////////////////////////////
Component* stel_ui::createTopBar(void)
{
    top_bar_date_lbl = new Label("-", courierFont);	top_bar_date_lbl->setPos(2,2);
    top_bar_hour_lbl = new Label("-", courierFont);	top_bar_hour_lbl->setPos(110,2);
    top_bar_fps_lbl = new Label("-", courierFont);	top_bar_fps_lbl->setPos(core->screen_W-100,2);
    top_bar_fov_lbl = new Label("-", courierFont);	top_bar_fov_lbl->setPos(core->screen_W-220,2);
    top_bar_appName_lbl = new Label(APP_NAME);
    top_bar_appName_lbl->setPos(core->screen_W/2-top_bar_appName_lbl->getSizex()/2,2);
    top_bar_ctr = new FilledContainer();
    top_bar_ctr->reshape(0,0,core->screen_W,15);
    top_bar_ctr->addComponent(top_bar_date_lbl);
    top_bar_ctr->addComponent(top_bar_hour_lbl);
    top_bar_ctr->addComponent(top_bar_fps_lbl);
    top_bar_ctr->addComponent(top_bar_fov_lbl);
    top_bar_ctr->addComponent(top_bar_appName_lbl);
	return top_bar_ctr;
}

////////////////////////////////////////////////////////////////////////////////
void stel_ui::updateTopBar(void)
{
	top_bar_ctr->setVisible(core->FlagShowTopBar);
	if (!core->FlagShowTopBar) return;

	double jd = core->navigation->get_JDay();

	if (core->FlagShowDate)
	{
		if (core->FlagUTC_Time)
			top_bar_date_lbl->setLabel(core->observatory->get_printable_date_UTC(jd));
		else
			top_bar_date_lbl->setLabel(core->observatory->get_printable_date_local(jd));
		top_bar_date_lbl->adjustSize();
	}
	top_bar_date_lbl->setVisible(core->FlagShowDate);

	if (core->FlagShowTime)
	{
	    if (core->FlagUTC_Time)
			top_bar_hour_lbl->setLabel(core->observatory->get_printable_time_UTC(jd) + " (UTC)");
	    else
			top_bar_hour_lbl->setLabel(core->observatory->get_printable_time_local(jd));
		top_bar_hour_lbl->adjustSize();
	}
	top_bar_hour_lbl->setVisible(core->FlagShowTime);

	top_bar_appName_lbl->setVisible(core->FlagShowAppName);

	char str[30];
    if (core->FlagShowFov)
	{
		sprintf(str,"fov=%2.3f\6", core->projection->get_fov());
		top_bar_fov_lbl->setLabel(str);
		top_bar_fov_lbl->adjustSize();
	}
	top_bar_fov_lbl->setVisible(core->FlagShowFov);

    if (core->FlagShowFps)
	{
		sprintf(str,"FPS:%4.2f",core->fps);
    	top_bar_fps_lbl->setLabel(str);
		top_bar_fps_lbl->adjustSize();
	}
	top_bar_fps_lbl->setVisible(core->FlagShowFps);
}

// Create the button panel in the lower left corner
Component* stel_ui::createFlagButtons(void)
{
	bt_flag_constellation_draw = new FlagButton(core->FlagConstellationDrawing, NULL, "bt_constellations");
	bt_flag_constellation_draw->setOnPressCallback(callback<void>(this, &stel_ui::cb));
	bt_flag_constellation_draw->setOnMouseInOutCallback(callback<void>(this, &stel_ui::cbr));

	bt_flag_constellation_name = new FlagButton(core->FlagConstellationName, NULL, "bt_const_names");
	bt_flag_constellation_name->setOnPressCallback(callback<void>(this, &stel_ui::cb));
	bt_flag_constellation_name->setOnMouseInOutCallback(callback<void>(this, &stel_ui::cbr));

	bt_flag_azimuth_grid = new FlagButton(core->FlagAzimutalGrid, NULL, "bt_grid");
	bt_flag_azimuth_grid->setOnPressCallback(callback<void>(this, &stel_ui::cb));
	bt_flag_azimuth_grid->setOnMouseInOutCallback(callback<void>(this, &stel_ui::cbr));

	bt_flag_equator_grid = new FlagButton(core->FlagEquatorialGrid, NULL, "bt_grid");
	bt_flag_equator_grid->setOnPressCallback(callback<void>(this, &stel_ui::cb));
	bt_flag_equator_grid->setOnMouseInOutCallback(callback<void>(this, &stel_ui::cbr));

	bt_flag_ground = new FlagButton(core->FlagGround, NULL, "bt_ground");
	bt_flag_ground->setOnPressCallback(callback<void>(this, &stel_ui::cb));
	bt_flag_ground->setOnMouseInOutCallback(callback<void>(this, &stel_ui::cbr));

	bt_flag_cardinals = new FlagButton(core->FlagCardinalPoints, NULL, "bt_cardinal");
	bt_flag_cardinals->setOnPressCallback(callback<void>(this, &stel_ui::cb));
	bt_flag_cardinals->setOnMouseInOutCallback(callback<void>(this, &stel_ui::cbr));

	bt_flag_atmosphere = new FlagButton(core->FlagAtmosphere, NULL, "bt_atmosphere");
	bt_flag_atmosphere->setOnPressCallback(callback<void>(this, &stel_ui::cb));
	bt_flag_atmosphere->setOnMouseInOutCallback(callback<void>(this, &stel_ui::cbr));

	bt_flag_nebula_name = new FlagButton(core->FlagNebulaName, NULL, "bt_nebula");
	bt_flag_nebula_name->setOnPressCallback(callback<void>(this, &stel_ui::cb));
	bt_flag_nebula_name->setOnMouseInOutCallback(callback<void>(this, &stel_ui::cbr));

	bt_flag_help = new FlagButton(core->FlagHelp, NULL, "bt_help");
	bt_flag_help->setOnPressCallback(callback<void>(this, &stel_ui::cb));
	bt_flag_help->setOnMouseInOutCallback(callback<void>(this, &stel_ui::cbr));

	bt_flag_equatorial_mode = new FlagButton(core->navigation->get_viewing_mode()==VIEW_EQUATOR, NULL, "bt_follow");
	bt_flag_equatorial_mode->setOnPressCallback(callback<void>(this, &stel_ui::cb));
	bt_flag_equatorial_mode->setOnMouseInOutCallback(callback<void>(this, &stel_ui::cbr));

	bt_flag_config = new FlagButton(core->FlagConfig, NULL, "bt_config");
	bt_flag_config->setOnPressCallback(callback<void>(this, &stel_ui::cb));
	bt_flag_config->setOnMouseInOutCallback(callback<void>(this, &stel_ui::cbr));

	bt_flag_quit = new FlagButton(true, NULL, "bt_quit");
	bt_flag_quit->setOnPressCallback(callback<void>(this, &stel_ui::cb));
	bt_flag_quit->setOnMouseInOutCallback(callback<void>(this, &stel_ui::cbr));

	bt_flag_ctr = new FilledContainer();
	bt_flag_ctr->addComponent(bt_flag_constellation_draw); 	bt_flag_constellation_draw->setPos(0,0);
	bt_flag_ctr->addComponent(bt_flag_constellation_name);	bt_flag_constellation_name->setPos(25,0);
	bt_flag_ctr->addComponent(bt_flag_azimuth_grid); 	bt_flag_azimuth_grid->setPos(50,0);
	bt_flag_ctr->addComponent(bt_flag_equator_grid);	bt_flag_equator_grid->setPos(75,0);
	bt_flag_ctr->addComponent(bt_flag_ground);			bt_flag_ground->setPos(100,0);
	bt_flag_ctr->addComponent(bt_flag_cardinals);		bt_flag_cardinals->setPos(125,0);
	bt_flag_ctr->addComponent(bt_flag_atmosphere);		bt_flag_atmosphere->setPos(150,0);
	bt_flag_ctr->addComponent(bt_flag_nebula_name);		bt_flag_nebula_name->setPos(175,0);
	bt_flag_ctr->addComponent(bt_flag_help);			bt_flag_help->setPos(200,0);
	bt_flag_ctr->addComponent(bt_flag_equatorial_mode);	bt_flag_equatorial_mode->setPos(225,0);
	bt_flag_ctr->addComponent(bt_flag_config);			bt_flag_config->setPos(250,0);
	bt_flag_ctr->addComponent(bt_flag_quit);			bt_flag_quit->setPos(275,0);

	bt_flag_ctr->setOnMouseInOutCallback(callback<void>(this, &stel_ui::bt_flag_ctrOnMouseInOut));
	bt_flag_ctr->reshape(0, core->screen_H-25, 12*25 -1, 25);

	return bt_flag_ctr;

}

////////////////////////////////////////////////////////////////////////////////
void stel_ui::cb(void)
{
	core->FlagConstellationDrawing 	= bt_flag_constellation_draw->getState();
	core->FlagConstellationName 	= bt_flag_constellation_name->getState();
	core->FlagAzimutalGrid 		= bt_flag_azimuth_grid->getState();
	core->FlagEquatorialGrid 	= bt_flag_equator_grid->getState();
	core->FlagGround	 		= bt_flag_ground->getState();
	core->FlagCardinalPoints	= bt_flag_cardinals->getState();
	core->FlagAtmosphere 		= bt_flag_atmosphere->getState();
	if (!core->FlagAtmosphere) core->tone_converter->set_world_adaptation_luminance(3.75f);
	core->FlagNebulaName		= bt_flag_nebula_name->getState();
	core->FlagHelp = bt_flag_help->getState();
	help_win->setVisible(core->FlagHelp);
	core->navigation->set_viewing_mode(bt_flag_equatorial_mode->getState() ? VIEW_EQUATOR : VIEW_HORIZON);
	core->FlagConfig			= bt_flag_config->getState();
	config_win->setVisible(core->FlagConfig);
	if (!bt_flag_quit->getState()) core->quit();
}

void stel_ui::bt_flag_ctrOnMouseInOut(void)
{
	if (bt_flag_ctr->getIsMouseOver()) bt_flag_help_lbl->setVisible(1);
	else bt_flag_help_lbl->setVisible(0);
}

void stel_ui::cbr(void)
{
	if (bt_flag_constellation_draw->getIsMouseOver())
		bt_flag_help_lbl->setLabel("Drawing of the Constellations [C]");
	if (bt_flag_constellation_name->getIsMouseOver())
		bt_flag_help_lbl->setLabel("Names of the Constellations [V]");
	if (bt_flag_azimuth_grid->getIsMouseOver())
		bt_flag_help_lbl->setLabel("Azimutal Grid [Z]");
	if (bt_flag_equator_grid->getIsMouseOver())
		bt_flag_help_lbl->setLabel("Equatorial Grid [E]");
	if (bt_flag_ground->getIsMouseOver())
		bt_flag_help_lbl->setLabel("Ground [G]");
	if (bt_flag_cardinals->getIsMouseOver())
		bt_flag_help_lbl->setLabel("Cardinal Points [Q]");
	if (bt_flag_atmosphere->getIsMouseOver())
		bt_flag_help_lbl->setLabel("Atmosphere [A]");
	if (bt_flag_nebula_name->getIsMouseOver())
		bt_flag_help_lbl->setLabel("Nebulas [N]");
	if (bt_flag_help->getIsMouseOver())
		bt_flag_help_lbl->setLabel("Help [H]");
	if (bt_flag_equatorial_mode->getIsMouseOver())
		bt_flag_help_lbl->setLabel("Equatorial/Altazimutal Mount [ENTER]");
	if (bt_flag_config->getIsMouseOver())
		bt_flag_help_lbl->setLabel("Configuration window");
	if (bt_flag_quit->getIsMouseOver())
		bt_flag_help_lbl->setLabel("Quit [CTRL + Q]");
}



// The window containing the info (licence)
Component* stel_ui::createLicenceWindow(void)
{
	licence_txtlbl = new TextLabel(
"                 \1   " APP_NAME "  Mars 2004  \1\n\
 \n\
\1   Copyright (c) 2000-2004 Fabien Chereau\n\
 \n\
\1   Please check last version and send bug report & comments\n\n\
on stellarium web page : http://stellarium.free.fr\n\n\
 \n\
\1   This program is free software; you can redistribute it and/or\n\
modify it under the terms of the GNU General Public License\n\
as published by the Free Software Foundation; either version 2\n\
of the License, or (at your option) any later version.\n\
 \n\
This program is distributed in the hope that it will be useful, but\n\
WITHOUT ANY WARRANTY; without even the implied\n\
warranty of MERCHANTABILITY or FITNESS FOR A\n\
PARTICULAR PURPOSE.  See the GNU General Public\n\
License for more details.\n\
 \n\
You should have received a copy of the GNU General Public\n\
License along with this program; if not, write to the\n\
Free Software Foundation, Inc., 59 Temple Place - Suite 330\n\
Boston, MA  02111-1307, USA.\n");
	licence_txtlbl->adjustSize();
	licence_txtlbl->setPos(10,10);
	licence_win = new StdBtWin("Infos");
	licence_win->reshape(300,200,400,350);
	licence_win->addComponent(licence_txtlbl);
	licence_win->setVisible(core->FlagInfos);

	return licence_win;
}


Component* stel_ui::createHelpWindow(void)
{
	help_txtlbl = new TextLabel(
"4 Directions     : Deplacement RA/DE\n\
Page Up/Down     : Zoom\n\
CTRL+Up/Down     : Zoom\n\
Left Click       : Select Star\n\
Right Click      : Clear Pointer\n\
CTRL+Left Click  : Clear Pointer\n\
SPACE : Center On Selected Object\n\
ENTER : Equatorial/Altazimutal mount\n\
CTRL + S : Take a Screenshot\n\
C   : Drawing of the Constellations\n\
V   : Names of the Constellations\n\
E   : Equatorial Grid\n\
Z   : Azimutal Grid\n\
N   : Nebulas\n\
P   : Planet Finder\n\
G   : Ground\n\
F   : Fog\n\
Q   : Cardinal Points\n\
A   : Atmosphere\n\
H   : Help\n\
4   : Ecliptic Line\n\
5   : Equator Line\n\
T   : Object Tracking\n\
S   : Stars\n\
I   : About Stellarium\n\
F1  : Toggle fullscreen if possible.\n\
CTRL+Q : Quit\n"
    ,courierFont);

	help_txtlbl->adjustSize();
	help_txtlbl->setPos(10,10);
	help_win = new StdBtWin("Help");
	help_win->reshape(300,200,400,380);
	help_win->addComponent(help_txtlbl);
    help_win->setVisible(core->FlagHelp);
	help_win->setOnHideBtCallback(callback<void>(this, &stel_ui::help_win_hideBtCallback));
	return help_win;
}

void stel_ui::help_win_hideBtCallback(void)
{
	help_win->setVisible(0);
}

/**********************************************************************************/
stel_ui::~stel_ui()
{
    if (desktop) delete desktop; 		desktop = NULL;
    if (spaceFont) delete spaceFont; 	spaceFont = NULL;
	if (baseTex) delete baseTex; 		baseTex = NULL;
}

/*******************************************************************/
void stel_ui::draw(void)
{
	// Special cool text transparency mode
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_BLEND);

	core->projection->set_2Dfullscreen_projection();	// 2D coordinate
	Component::enableScissor();

    glScalef(1, -1, 1);						// invert the y axis, down is positive
    glTranslatef(0, -core->screen_H, 0);	// move the origin from the bottom left corner to the upper left corner

	desktop->draw();

	Component::disableScissor();
    core->projection->restore_from_2Dfullscreen_projection();	// Restore the other coordinate
}

/*******************************************************************************/
int stel_ui::handle_move(int x, int y)
{
	return desktop->onMove(x, y);
}

/*******************************************************************************/
int stel_ui::handle_clic(Uint16 x, Uint16 y, Uint8 button, Uint8 state)
{   // Convert the name from GLU to my GUI
    enum S_GUI_VALUE bt;
    enum S_GUI_VALUE st;
    switch (button)
    {   case SDL_BUTTON_RIGHT : bt=S_GUI_MOUSE_RIGHT; break;
        case SDL_BUTTON_LEFT : bt=S_GUI_MOUSE_LEFT; break;
        case SDL_BUTTON_MIDDLE : bt=S_GUI_MOUSE_MIDDLE; break;
        default : bt=S_GUI_MOUSE_LEFT;
    }
    if (state==SDL_RELEASED) st=S_GUI_RELEASED; else st=S_GUI_PRESSED;

    // Send the mouse event to the User Interface
    if (desktop->onClic((int)x, (int)y, bt, st))
    {
        return 1;
    }
    else
    // Manage the event for the main window
    {
		if (state==SDL_RELEASED) return 1;
        // Deselect the selected object
        if (button==SDL_BUTTON_RIGHT)
        {
			core->selected_object=NULL;
            return 1;
        }
        if (button==SDL_BUTTON_MIDDLE)
        {
			if (core->selected_object)
            {
				core->navigation->move_to(core->selected_object->get_earth_equ_pos(core->navigation),
					core->auto_move_duration);
            }
        }
        if (button==SDL_BUTTON_LEFT)
        {   
        	// CTRL + left clic = right clic for 1 button mouse
			if (SDL_GetModState() & KMOD_CTRL)
			{
				core->selected_object=NULL;
        	}

        	// Left clic -> selection of an object
			stel_object* tempselect= core->clever_find((int)x, core->screen_H-(int)y);

			// Unselect on second clic on the same object
			if (core->selected_object!=NULL && core->selected_object==tempselect)
			{
				core->selected_object = NULL;
			}
			else
			{
				core->selected_object = core->clever_find((int)x, core->screen_H-(int)y);
            }

	    // If an object has been found
	    if (core->selected_object)
            {
	      updateInfoSelectString();
	      // If an object was selected keep the earth following
	      if (core->navigation->get_flag_traking()) core->navigation->set_flag_lock_equ_pos(1);
	      core->navigation->set_flag_traking(0);

	      if (core->selected_object->get_type()==STEL_OBJECT_STAR) {
		core->selected_constellation=core->asterisms->is_star_in((Hip_Star*)core->selected_object);
	      } else {
		core->selected_constellation=NULL;
	      }

	      if (core->selected_object->get_type()==STEL_OBJECT_PLANET) {
		core->selected_planet=(planet*)core->selected_object;
	      } else {
		core->selected_planet=NULL;
	      }

            } else {
	      core->selected_constellation=NULL;
	      core->selected_planet=NULL;
	    }
	}
    }
	return 0;
}


/*******************************************************************************/
int stel_ui::handle_keys(SDLKey key, S_GUI_VALUE state)
{
	desktop->onKey(key, state);

	if (state==S_GUI_PRESSED)
    {
    	if(key==SDLK_q)
    	{
			if (SDL_GetModState() & KMOD_CTRL) core->quit();
		}
        if(key==SDLK_c)
        {
        	core->FlagConstellationDrawing=!core->FlagConstellationDrawing;
		}
        if(key==SDLK_d)
        {
        	core->FlagStarName=!core->FlagStarName;
		}
        if(key==SDLK_1)
        {
        	core->FlagConfig=!core->FlagConfig;
			config_win->setVisible(core->FlagConfig);
		}
        if(key==SDLK_p)
        {	
	  if(!core->FlagPlanetsHints) {
	    core->FlagPlanetsHints=1;
	  } else if( !core->FlagPlanetsOrbits) {
	    core->FlagPlanetsOrbits = 1;
	  } else {
	    core->FlagPlanetsOrbits = 0;
	    core->FlagPlanetsHints = 0;
	  }
	}
        if(key==SDLK_v)
        {
        	core->FlagConstellationName=!core->FlagConstellationName;
		}
        if(key==SDLK_z)
        {	
        	core->FlagAzimutalGrid=!core->FlagAzimutalGrid;
		}
        if(key==SDLK_e)
        {	
        	core->FlagEquatorialGrid=!core->FlagEquatorialGrid;
		}
        if(key==SDLK_n)
        {	
        	core->FlagNebulaName=!core->FlagNebulaName;
		}
        if(key==SDLK_g)
        {	
        	core->FlagGround=!core->FlagGround;
		}
        if(key==SDLK_f)
        {	
        	core->FlagFog=!core->FlagFog;
		}
        if(key==SDLK_q)
        {	
        	core->FlagCardinalPoints=!core->FlagCardinalPoints;
		}
        if(key==SDLK_a)
        {	
        	core->FlagAtmosphere=!core->FlagAtmosphere;
	}
        if(key==SDLK_h)
        {	
        	core->FlagHelp=!core->FlagHelp;
			help_win->setVisible(core->FlagHelp);
		}
		if(key==SDLK_COMMA || key==SDLK_4)
        {
        	core->FlagEclipticLine=!core->FlagEclipticLine;
		}
        if(key==SDLK_PERIOD || key==SDLK_5)
        {
        	core->FlagEquatorLine=!core->FlagEquatorLine;
		}

        if(key==SDLK_t)
        {
			core->navigation->set_flag_lock_equ_pos(!core->navigation->get_flag_lock_equ_pos());
		}
        if(key==SDLK_s && !(SDL_GetModState() & KMOD_CTRL))
        {	
        	core->FlagStars=!core->FlagStars;
		}
        if(key==SDLK_SPACE)
        {	
        	if (core->selected_object)
			{
				core->navigation->move_to(core->selected_object->get_earth_equ_pos(core->navigation),
					core->auto_move_duration);
				core->navigation->set_flag_traking(1);
			}
		}
        if(key==SDLK_i)
        {
        	core->FlagInfos=!core->FlagInfos;
            licence_win->setVisible(core->FlagInfos);
		}
        if(key==SDLK_EQUALS)
        {
        	core->navigation->set_JDay(core->navigation->get_JDay()+JD_DAY);
		}
        if(key==SDLK_MINUS)
        {
        	core->navigation->set_JDay(core->navigation->get_JDay()-JD_DAY);
		}
        if(key==SDLK_m)
        {
        	if (core->FlagEnableTuiMenu) core->FlagShowTuiMenu = true;
		}
        if(key==SDLK_k)
        {
        	core->navigation->set_time_speed(JD_SECOND);
		}
        if(key==SDLK_o)
        {
			if (core->ssystem->get_moon()->get_sphere_scale()==1.f)
        		core->ssystem->get_moon()->set_sphere_scale(core->moon_scale);
			else
				core->ssystem->get_moon()->set_sphere_scale(1.f);
		}
        if(key==SDLK_l)
        {
			double s = core->navigation->get_time_speed();
			if (s>=JD_SECOND) s*=10.;
			else if (s<-JD_SECOND) s/=10.;
			else if (s>=0. && s<JD_SECOND) s=JD_SECOND;
			else if (s>=-JD_SECOND && s<0.) s=0.;
        	core->navigation->set_time_speed(s);
		}
        if(key==SDLK_j)
        {
			double s = core->navigation->get_time_speed();
			if (s>JD_SECOND) s/=10.;
			else if (s<=-JD_SECOND) s*=10.;
			else if (s>-JD_SECOND && s<=0.) s=-JD_SECOND;
			else if (s>0. && s<=JD_SECOND) s=0.;
        	core->navigation->set_time_speed(s);
		}
        if(key==SDLK_6)
        {
			core->FlagTimePause = !core->FlagTimePause;
			if (core->FlagTimePause)
			{
				core->temp_time_velocity = core->navigation->get_time_speed();
        		core->navigation->set_time_speed(0);
			}
			else
			{
				core->navigation->set_time_speed(core->temp_time_velocity);
			}
		}
        if(key==SDLK_7)
        {
        	core->navigation->set_time_speed(0);
	}
	if(key==SDLK_8)
	  {
	    // set time to default, either actual or preset time based on settings
	    if (core->StartupTimeMode=="preset" || core->StartupTimeMode=="Preset")
	      {
		core->navigation->set_JDay(core->PresetSkyTime -
					   core->observatory->get_GMT_shift(core->PresetSkyTime) * JD_HOUR);
	      }
	    else
	      {
		core->navigation->set_JDay(get_julian_from_sys());
	      }
	  }

	if(key==SDLK_9) {
	  int zhr = core->meteors->get_ZHR();

	  if(zhr <= 10 ) {
	    core->meteors->set_ZHR( 80 );  // standard Perseids rate
	  } else if( zhr <= 80 ) {
	    core->meteors->set_ZHR( 10000 );  // exceptional Leonid rate
	  } else if( zhr <= 10000 ) {
	    core->meteors->set_ZHR( 144000 );  // highest ever recorded ZHR (1966 Leonids)
	  } else {
	    core->meteors->set_ZHR( 10 );  // set to ***default base rate (10 is normal, 0 would be none)
	  }
	}

        if(key==SDLK_LEFTBRACKET)
        {
        	core->navigation->set_JDay(core->navigation->get_JDay()-7*JD_DAY);
		}
        if(key==SDLK_RIGHTBRACKET)
        {
        	core->navigation->set_JDay(core->navigation->get_JDay()+7*JD_DAY);
		}

		if(key==SDLK_SLASH)
        {
			if (SDL_GetModState() & KMOD_CTRL)
			{
				core->auto_zoom_out(core->auto_move_duration);
			}
			else core->auto_zoom_in(core->auto_move_duration);
		}
		if(key==SDLK_BACKSLASH)
		{
			core->auto_zoom_out(core->auto_move_duration);
		}
		if(key==SDLK_x)
        {
			core->FlagShowTuiDateTime = !core->FlagShowTuiDateTime;
			core->FlagShowTuiShortInfo = core->FlagShowTuiDateTime;
		}
		if(key==SDLK_r)
        {
			core->FlagConstellationArt = !core->FlagConstellationArt;
		}
		if(key==SDLK_4)
        {
			core->FlagConstellationArt = !core->FlagConstellationArt;
		}
		if(key==SDLK_RETURN)
        {
			core->navigation->switch_viewing_mode();
		}
    }
    return 0;
}


// Update changing values
void stel_ui::gui_update_widgets(void)
{
	updateTopBar();

	// To prevent a minor bug
	if (!core->FlagShowSelectedObjectInfos || !core->selected_object) info_select_ctr->setVisible(0);
	else if (core->selected_object) info_select_ctr->setVisible(1);
	bt_flag_ctr->setVisible(core->FlagMenu);

	bt_flag_constellation_draw->setState(core->FlagConstellationDrawing);
	bt_flag_constellation_name->setState(core->FlagConstellationName);
	bt_flag_azimuth_grid->setState(core->FlagAzimutalGrid);
	bt_flag_equator_grid->setState(core->FlagEquatorialGrid);
	bt_flag_ground->setState(core->FlagGround);
	bt_flag_cardinals->setState(core->FlagCardinalPoints);
	bt_flag_atmosphere->setState(core->FlagAtmosphere);
	bt_flag_nebula_name->setState(core->FlagNebulaName);
	bt_flag_help->setState(help_win->getVisible());
	bt_flag_equatorial_mode->setState(core->navigation->get_viewing_mode()==VIEW_EQUATOR);
	bt_flag_config->setState(config_win->getVisible());

	if (config_win->getVisible()) updateConfigForm();
}


// Update the infos about the selected object in the TextLabel widget
void stel_ui::updateInfoSelectString(void)
{
	static char objectInfo[300];
    objectInfo[0]='\0';
	core->selected_object->get_info_string(objectInfo, core->navigation);
    info_select_txtlbl->setLabel(objectInfo);
    if (core->FlagShowSelectedObjectInfos) info_select_ctr->setVisible(1);
	if (core->selected_object->get_type()==STEL_OBJECT_NEBULA)
		info_select_txtlbl->setTextColor(core->NebulaLabelColor);
	if (core->selected_object->get_type()==STEL_OBJECT_PLANET)
		info_select_txtlbl->setTextColor(core->PlanetNamesColor);
	if (core->selected_object->get_type()==STEL_OBJECT_STAR)
		info_select_txtlbl->setTextColor(core->selected_object->get_RGB());
}

