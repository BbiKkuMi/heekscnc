// Profile.cpp

#include "stdafx.h"
#include "Profile.h"
#include "CNCConfig.h"
#include "ProgramCanvas.h"
#include "interface/HeeksObj.h"
#include "interface/PropertyDouble.h"
#include "interface/PropertyChoice.h"
#include "interface/PropertyVertex.h"
#include "interface/PropertyCheck.h"
#include "tinyxml/tinyxml.h"
#include "interface/Tool.h"

#include <sstream>

CProfileParams::CProfileParams()
{
	m_tool_diameter = 0.0;
	m_clearance_height = 0.0;
	m_final_depth = 0.0;
	m_rapid_down_to_height = 0.0;
	m_horizontal_feed_rate = 0.0;
	m_vertical_feed_rate = 0.0;
	m_spindle_speed = 0.0;
	m_tool_on_side = 0;
	m_auto_roll_on = true;
	m_auto_roll_off = true;
	m_roll_on_point[0] = m_roll_on_point[1] = m_roll_on_point[2] = 0.0;
	m_roll_off_point[0] = m_roll_off_point[1] = m_roll_off_point[2] = 0.0;
	m_start_given = false;
	m_end_given = false;
	m_start[0] = m_start[1] = m_start[2] = 0.0;
	m_end[0] = m_end[1] = m_end[2] = 0.0;
}

void CProfileParams::set_initial_values()
{
	CNCConfig config;
	config.Read(_T("ProfileToolDiameter"), &m_tool_diameter, 3.0);
	config.Read(_T("ProfileClearanceHeight"), &m_clearance_height, 5.0);
	config.Read(_T("ProfileFinalDepth"), &m_final_depth, -0.1);
	config.Read(_T("ProfileRapidDown"), &m_rapid_down_to_height, 2.0);
	config.Read(_T("ProfileHorizFeed"), &m_horizontal_feed_rate, 100.0);
	config.Read(_T("ProfileVertFeed"), &m_vertical_feed_rate, 100.0);
	config.Read(_T("ProfileSpindleSpeed"), &m_spindle_speed, 7000);
	config.Read(_T("ProfileToolOnSide"), &m_tool_on_side, 1);
}

void CProfileParams::write_values_to_config()
{
	CNCConfig config;
	config.Write(_T("ProfileToolDiameter"), m_tool_diameter);
	config.Write(_T("ProfileClearanceHeight"), m_clearance_height);
	config.Write(_T("ProfileFinalDepth"), m_final_depth);
	config.Write(_T("ProfileRapidDown"), m_rapid_down_to_height);
	config.Write(_T("ProfileHorizFeed"), m_horizontal_feed_rate);
	config.Write(_T("ProfileVertFeed"), m_vertical_feed_rate);
	config.Write(_T("ProfileSpindleSpeed"), m_spindle_speed);
	config.Write(_T("ProfileToolOnSide"), m_tool_on_side);
}

static void on_set_tool_diameter(double value, HeeksObj* object){((CProfile*)object)->m_params.m_tool_diameter = value;}
static void on_set_clearance_height(double value, HeeksObj* object){((CProfile*)object)->m_params.m_clearance_height = value;}
static void on_set_final_depth(double value, HeeksObj* object){((CProfile*)object)->m_params.m_final_depth = value;}
static void on_set_rapid_down_to_height(double value, HeeksObj* object){((CProfile*)object)->m_params.m_rapid_down_to_height = value;}
static void on_set_horizontal_feed_rate(double value, HeeksObj* object){((CProfile*)object)->m_params.m_horizontal_feed_rate = value;}
static void on_set_vertical_feed_rate(double value, HeeksObj* object){((CProfile*)object)->m_params.m_vertical_feed_rate = value;}
static void on_set_spindle_speed(double value, HeeksObj* object){((CProfile*)object)->m_params.m_spindle_speed = value;}
static void on_set_tool_on_side(int value, HeeksObj* object){
	switch(value)
	{
	case 0:
		((CProfile*)object)->m_params.m_tool_on_side = 1;
		break;
	case 1:
		((CProfile*)object)->m_params.m_tool_on_side = -1;
		break;
	default:
		((CProfile*)object)->m_params.m_tool_on_side = 0;
		break;
	}
}
static void on_set_auto_roll_on(bool value, HeeksObj* object){((CProfile*)object)->m_params.m_auto_roll_on = value; heeksCAD->RefreshProperties();}
static void on_set_roll_on_point(const double* vt, HeeksObj* object){memcpy(((CProfile*)object)->m_params.m_roll_on_point, vt, 3*sizeof(double));}
static void on_set_auto_roll_off(bool value, HeeksObj* object){((CProfile*)object)->m_params.m_auto_roll_off = value; heeksCAD->RefreshProperties();}
static void on_set_roll_off_point(const double* vt, HeeksObj* object){memcpy(((CProfile*)object)->m_params.m_roll_off_point, vt, 3*sizeof(double));}
static void on_set_start_given(bool value, HeeksObj* object){((CProfile*)object)->m_params.m_start_given = value; heeksCAD->RefreshProperties();}
static void on_set_start(const double* vt, HeeksObj* object){memcpy(((CProfile*)object)->m_params.m_start, vt, 3*sizeof(double));}
static void on_set_end_given(bool value, HeeksObj* object){((CProfile*)object)->m_params.m_end_given = value; heeksCAD->RefreshProperties();}
static void on_set_end(const double* vt, HeeksObj* object){memcpy(((CProfile*)object)->m_params.m_end, vt, 3*sizeof(double));}

void CProfileParams::GetProperties(CProfile* parent, std::list<Property *> *list)
{
	list->push_back(new PropertyDouble(_("tool diameter"), m_tool_diameter, parent, on_set_tool_diameter));
	list->push_back(new PropertyDouble(_("clearance height"), m_clearance_height, parent, on_set_clearance_height));
	list->push_back(new PropertyDouble(_("final depth"), m_final_depth, parent, on_set_final_depth));
	list->push_back(new PropertyDouble(_("rapid down to height"), m_rapid_down_to_height, parent, on_set_rapid_down_to_height));
	list->push_back(new PropertyDouble(_("horizontal feed rate"), m_horizontal_feed_rate, parent, on_set_horizontal_feed_rate));
	list->push_back(new PropertyDouble(_("vertical feed rate"), m_vertical_feed_rate, parent, on_set_vertical_feed_rate));
	list->push_back(new PropertyDouble(_("spindle speed"), m_spindle_speed, parent, on_set_spindle_speed));
	{
		std::list< wxString > choices;
		choices.push_back(_("Left"));
		choices.push_back(_("Right"));
		choices.push_back(_("On"));
		int choice = 0;
		if(m_tool_on_side == -1)choice = 1;
		else if(m_tool_on_side == 0)choice = 2;
		list->push_back(new PropertyChoice(_("tool on side"), choices, choice, parent, on_set_tool_on_side));
	}

	if(parent->m_sketches.size() == 1) // multiple sketches must use auto roll on, and can not have start and end points specified
	{
		list->push_back(new PropertyCheck(_("auto roll on"), m_auto_roll_on, parent, on_set_auto_roll_on));
		if(!m_auto_roll_on)list->push_back(new PropertyVertex(_("roll on point"), m_roll_on_point, parent, on_set_roll_on_point));
		list->push_back(new PropertyCheck(_("auto roll off"), m_auto_roll_off, parent, on_set_auto_roll_off));
		if(!m_auto_roll_off)list->push_back(new PropertyVertex(_("roll off point"), m_roll_off_point, parent, on_set_roll_off_point));
		list->push_back(new PropertyCheck(_("use start point"), m_start_given, parent, on_set_start_given));
		if(m_start_given)list->push_back(new PropertyVertex(_("start point"), m_start, parent, on_set_start));
		list->push_back(new PropertyCheck(_("use end point"), m_end_given, parent, on_set_end_given));
		if(m_end_given)list->push_back(new PropertyVertex(_("end point"), m_end, parent, on_set_end));
	}
}

void CProfileParams::WriteXMLAttributes(TiXmlNode *root)
{
	TiXmlElement * element;
	element = new TiXmlElement( "params" );
	root->LinkEndChild( element );  
	element->SetDoubleAttribute("toold", m_tool_diameter);
	element->SetDoubleAttribute("clear", m_clearance_height);
	element->SetDoubleAttribute("depth", m_final_depth);
	element->SetDoubleAttribute("r", m_rapid_down_to_height);
	element->SetDoubleAttribute("hfeed", m_horizontal_feed_rate);
	element->SetDoubleAttribute("vfeed", m_vertical_feed_rate);
	element->SetDoubleAttribute("spin", m_spindle_speed);
	element->SetDoubleAttribute("side", m_tool_on_side);
	element->SetAttribute("auto_roll_on", m_auto_roll_on ? 1:0);
	if(!m_auto_roll_on)
	{
		element->SetDoubleAttribute("roll_onx", m_roll_on_point[0]);
		element->SetDoubleAttribute("roll_ony", m_roll_on_point[1]);
		element->SetDoubleAttribute("roll_onz", m_roll_on_point[2]);
	}
	element->SetAttribute("auto_roll_off", m_auto_roll_off ? 1:0);
	if(!m_auto_roll_off)
	{
		element->SetDoubleAttribute("roll_offx", m_roll_off_point[0]);
		element->SetDoubleAttribute("roll_offy", m_roll_off_point[1]);
		element->SetDoubleAttribute("roll_offz", m_roll_off_point[2]);
	}
	element->SetAttribute("start_given", m_start_given ? 1:0);
	if(m_start_given)
	{
		element->SetDoubleAttribute("startx", m_start[0]);
		element->SetDoubleAttribute("starty", m_start[1]);
		element->SetDoubleAttribute("startz", m_start[2]);
	}
	element->SetAttribute("end_given", m_end_given ? 1:0);
	if(m_end_given)
	{
		element->SetDoubleAttribute("endx", m_end[0]);
		element->SetDoubleAttribute("endy", m_end[1]);
		element->SetDoubleAttribute("endz", m_end[2]);
	}
}

void CProfileParams::ReadFromXMLElement(TiXmlElement* pElem)
{
	int int_for_bool;

	pElem->Attribute("toold", &m_tool_diameter);
	pElem->Attribute("clear", &m_clearance_height);
	pElem->Attribute("depth", &m_final_depth);
	pElem->Attribute("r", &m_rapid_down_to_height);
	pElem->Attribute("hfeed", &m_horizontal_feed_rate);
	pElem->Attribute("vfeed", &m_vertical_feed_rate);
	pElem->Attribute("spin", &m_spindle_speed);
	pElem->Attribute("side", &m_tool_on_side);
	pElem->Attribute("auto_roll_on", &int_for_bool); m_auto_roll_on = (int_for_bool != 0);
	pElem->Attribute("roll_onx", &m_roll_on_point[0]);
	pElem->Attribute("roll_ony", &m_roll_on_point[1]);
	pElem->Attribute("roll_onz", &m_roll_on_point[2]);
	pElem->Attribute("auto_roll_off", &int_for_bool); m_auto_roll_off = (int_for_bool != 0);
	pElem->Attribute("roll_offx", &m_roll_off_point[0]);
	pElem->Attribute("roll_offy", &m_roll_off_point[1]);
	pElem->Attribute("roll_offz", &m_roll_off_point[2]);
	pElem->Attribute("start_given", &int_for_bool); m_start_given = (int_for_bool != 0);
	pElem->Attribute("startx", &m_start[0]);
	pElem->Attribute("starty", &m_start[1]);
	pElem->Attribute("startz", &m_start[2]);
	pElem->Attribute("end_given", &int_for_bool); m_end_given = (int_for_bool != 0);
	pElem->Attribute("endx", &m_end[0]);
	pElem->Attribute("endy", &m_end[1]);
	pElem->Attribute("endz", &m_end[2]);
}

#define AUTO_ROLL_ON_OFF_SIZE 2.0

void CProfileParams::GetRollOnPos(HeeksObj* sketch, double &x, double &y)
{
	// roll on
	if(m_auto_roll_on)
	{
		if(sketch)
		{
			HeeksObj* first_child = sketch->GetAtIndex(0);
			if(first_child)
			{
				double s[3];
				if(!(first_child->GetStartPoint(s)))return;
				x = s[0];
				y = s[1];
				if(m_tool_on_side == 0)return;
				double v[3];
				if(heeksCAD->GetSegmentVector(first_child, 0.0, v))
				{
					double off_vec[3] = {-v[1], v[0], 0.0};
					if(m_tool_on_side == -1){off_vec[0] = -off_vec[0]; off_vec[1] = -off_vec[1];}
					x = s[0] + off_vec[0] * (m_tool_diameter/2 + AUTO_ROLL_ON_OFF_SIZE) - v[0] * AUTO_ROLL_ON_OFF_SIZE;
					y = s[1] + off_vec[1] * (m_tool_diameter/2 + AUTO_ROLL_ON_OFF_SIZE) - v[1] * AUTO_ROLL_ON_OFF_SIZE;
				}
			}
		}
	}
	else
	{
		x = m_roll_on_point[0];
		y = m_roll_on_point[1];
	}
}

void CProfileParams::GetRollOffPos(HeeksObj* sketch, double &x, double &y)
{
	// roll off
	if(m_auto_roll_off)
	{
			int num_spans = sketch->GetNumChildren();
			if(num_spans > 0)
			{
				HeeksObj* last_child = sketch->GetAtIndex(num_spans - 1);
				if(last_child)
				{
					double e[3];
					if(!(last_child->GetEndPoint(e)))return;
					x = e[0];
					y = e[1];
					if(m_tool_on_side == 0)return;
					double v[3];
					if(heeksCAD->GetSegmentVector(last_child, 0.0, v))
					{
						double off_vec[3] = {-v[1], v[0], 0.0};
						if(m_tool_on_side == -1){off_vec[0] = -off_vec[0]; off_vec[1] = -off_vec[1];}
						x = e[0] + off_vec[0] * (m_tool_diameter/2 + AUTO_ROLL_ON_OFF_SIZE) + v[0] * AUTO_ROLL_ON_OFF_SIZE;
						y = e[1] + off_vec[1] * (m_tool_diameter/2 + AUTO_ROLL_ON_OFF_SIZE) + v[1] * AUTO_ROLL_ON_OFF_SIZE;
					}
				}
			}
	}
	else
	{
		x = m_roll_off_point[0];
		y = m_roll_off_point[1];
	}
}

void CProfile::WriteSketchDefn(HeeksObj* sketch, int id_to_use)
{
	theApp.m_program_canvas->m_textCtrl->AppendText(wxString::Format(_T("k%d = kurve.new()\n"), id_to_use > 0 ? id_to_use : sketch->m_id));

	bool started = false;
	int sketch_id = (id_to_use > 0 ? id_to_use : sketch->m_id);

	for(HeeksObj* span_object = sketch->GetFirstChild(); span_object; span_object = sketch->GetNextChild())
	{
		double s[3] = {0, 0, 0};
		double e[3] = {0, 0, 0};
		double c[3] = {0, 0, 0};

		if(span_object){
			int type = span_object->GetType();
			if(type == LineType || type == ArcType)
			{
				if(!started)
				{
					span_object->GetStartPoint(s);
#ifdef UNICODE
					std::wostringstream ss;
#else
					std::ostringstream ss;
#endif
					ss.imbue(std::locale("C"));

					ss << "kurve.add_point(k" << sketch_id << ", 0, " << s[0] << ", " << s[1] << ", 0.0, 0.0)\n";
					theApp.m_program_canvas->m_textCtrl->AppendText(ss.str().c_str());
					started = true;
				}
				span_object->GetEndPoint(e);
				if(type == LineType)
				{
#ifdef UNICODE
					std::wostringstream ss;
#else
					std::ostringstream ss;
#endif
					ss.imbue(std::locale("C"));

					ss << "kurve.add_point(k" << sketch_id << ", 0, " << e[0] << ", " << e[1] << ", 0.0, 0.0)\n";
					theApp.m_program_canvas->m_textCtrl->AppendText(ss.str().c_str());
				}
				else if(type == ArcType)
				{
					span_object->GetCentrePoint(c);
					double pos[3];
					heeksCAD->GetArcAxis(span_object, pos);
					int span_type = (pos[2] >=0) ? 1:-1;
#ifdef UNICODE
					std::wostringstream ss;
#else
					std::ostringstream ss;
#endif
					ss.imbue(std::locale("C"));

					ss << "kurve.add_point(k" << sketch_id << ", " << span_type << ", " << e[0] << ", " << e[1] << ", " << c[0] << ", " << c[1] << ")\n";
					theApp.m_program_canvas->m_textCtrl->AppendText(ss.str().c_str());
				}
			}
		}
	}

	theApp.m_program_canvas->m_textCtrl->AppendText(_T("\n"));

	if(m_sketches.size() == 1 && (m_params.m_start_given || m_params.m_end_given))
	{
		wxString start_string;
		if(m_params.m_start_given)
		{
#ifdef UNICODE
			std::wostringstream ss;
#else
			std::ostringstream ss;
#endif
			ss << ", startx = " << m_params.m_start[0] << ", starty = " << m_params.m_start[1];
			start_string = ss.str().c_str();
		}

		wxString finish_string;
		if(m_params.m_end_given)
		{
#ifdef UNICODE
			std::wostringstream ss;
#else
			std::ostringstream ss;
#endif
			ss << ", finishx = " << m_params.m_end[0] << ", finishy = " << m_params.m_end[1];
			finish_string = ss.str().c_str();
		}

		theApp.m_program_canvas->m_textCtrl->AppendText(wxString::Format(_T("kurve_funcs.make_smaller( k%d%s%s)\n"), sketch_id, start_string.c_str(), finish_string.c_str()));
	}
}

void CProfile::AppendTextForOneSketch(HeeksObj* object, int sketch)
{
	if(object)
	{
		WriteSketchDefn(object, sketch);

		// start - assume we are at a suitable clearance height

		// get offset side
		wxString side_string;
		switch(m_params.m_tool_on_side)
		{
		case 1:
			side_string = _T("left");
			break;
		case -1:
			side_string = _T("right");
			break;
		default:
			side_string = _T("on");
			break;
		}

		// get roll on string
		wxString roll_on_string;
		if(m_params.m_tool_on_side)
		{
			if(m_params.m_auto_roll_on || (m_sketches.size() > 1))
			{
				theApp.m_program_canvas->m_textCtrl->AppendText(wxString::Format(_T("roll_on_x, roll_on_y = kurve_funcs.roll_on_point(k%d, '%s', tool_diameter/2)\n"), sketch, side_string.c_str()));
				roll_on_string = wxString(_T("roll_on_x, roll_on_y"));
			}
			else
			{
#ifdef UNICODE
				std::wostringstream ss;
#else
				std::ostringstream ss;
#endif
				ss.imbue(std::locale("C"));
				ss << m_params.m_roll_on_point[0] << ", " << m_params.m_roll_on_point[1];
				roll_on_string = ss.str().c_str();
			}
		}
		else
		{
			theApp.m_program_canvas->m_textCtrl->AppendText(wxString::Format(_T("sp, span1sx, span1sy, ex, ey, cx, cy = kurve.get_span(k%d, 0)\n"), sketch));
			roll_on_string = _T("span1sx, span1sy");
		}

		// rapid across to it
		theApp.m_program_canvas->m_textCtrl->AppendText(wxString::Format(_T("rapid(%s)\n"), roll_on_string.c_str()));

		// rapid down to just above the material
		theApp.m_program_canvas->m_textCtrl->AppendText(wxString(_T("rapid(z = rapid_down_to_height)\n")));

		// feed down to final depth
		theApp.m_program_canvas->m_textCtrl->AppendText(wxString(_T("feed(z = final_depth)\n")));			

		wxString roll_off_string;
		if(m_params.m_tool_on_side)
		{
			if(m_params.m_auto_roll_off || (m_sketches.size() > 1))
			{
				theApp.m_program_canvas->m_textCtrl->AppendText(wxString::Format(_T("roll_off_x, roll_off_y = kurve_funcs.roll_off_point(k%d, '%s', tool_diameter/2)\n"), sketch, side_string.c_str()));			
				roll_off_string = wxString(_T("roll_off_x, roll_off_y"));
			}
			else
			{
#ifdef UNICODE
				std::wostringstream ss;
#else
				std::ostringstream ss;
#endif
				ss << m_params.m_roll_off_point[0] << ", " << m_params.m_roll_off_point[1];
				roll_off_string = ss.str().c_str();
			}
		}
		else
		{
			theApp.m_program_canvas->m_textCtrl->AppendText(wxString::Format(_T("sp, sx, sy, ex, ey, cx, cy = kurve.get_span(k%d, kurve.num_spans(k%d) - 1)\n"), sketch, sketch));
			roll_off_string = _T("ex, ey");
		}

		// profile the kurve
		theApp.m_program_canvas->m_textCtrl->AppendText(wxString::Format(_T("kurve_funcs.profile(k%d, '%s', tool_diameter/2, %s, %s)\n"), sketch, side_string.c_str(), roll_on_string.c_str(), roll_off_string.c_str()));

		// rapid back up to clearance plane
		theApp.m_program_canvas->m_textCtrl->AppendText(wxString(_T("rapid(z = clearance)\n")));			
	}
}

void CProfile::AppendTextToProgram()
{
	{
#ifdef UNICODE
	std::wostringstream ss;
#else
    std::ostringstream ss;
#endif
    ss.imbue(std::locale("C"));

    ss << "clearance = float(" << m_params.m_clearance_height << ")\n";
    ss << "rapid_down_to_height = float(" << m_params.m_rapid_down_to_height << ")\n";
    ss << "final_depth = float(" << m_params.m_final_depth << ")\n";
    ss << "tool_diameter = float(" << m_params.m_tool_diameter << ")\n";
    ss << "spindle(" << m_params.m_spindle_speed << ")\n";
    ss << "feedrate(" << m_params.m_horizontal_feed_rate << ")\n";
    ss << "tool_change(1)\n";
	theApp.m_program_canvas->m_textCtrl->AppendText(ss.str().c_str());
	}

	for(std::list<int>::iterator It = m_sketches.begin(); It != m_sketches.end(); It++)
	{
		int sketch = *It;

		// write a kurve definition
		HeeksObj* object = heeksCAD->GetIDObject(SketchType, sketch);
		if(object == NULL || object->GetNumChildren() == 0)continue;

		HeeksObj* re_ordered_sketch = NULL;
		if(heeksCAD->GetSketchOrder(object) == SketchOrderTypeBad)
		{
			re_ordered_sketch = object->MakeACopy();
			heeksCAD->ReOrderSketch(re_ordered_sketch, SketchOrderTypeReOrder);
			object = re_ordered_sketch;
		}

		if(heeksCAD->GetSketchOrder(object) == SketchOrderTypeMultipleCurves)
		{
			std::list<HeeksObj*> new_separate_sketches;
			heeksCAD->ExtractSeparateSketches(object, new_separate_sketches);
			for(std::list<HeeksObj*>::iterator It = new_separate_sketches.begin(); It != new_separate_sketches.end(); It++)
			{
				HeeksObj* one_curve_sketch = *It;
				AppendTextForOneSketch(one_curve_sketch, sketch);
				delete one_curve_sketch;
			}
		}
		else
		{
			AppendTextForOneSketch(object, sketch);
		}

		if(re_ordered_sketch)
		{
			delete re_ordered_sketch;
		}
	}
}

static unsigned char cross16[32] = {0x80, 0x01, 0x40, 0x02, 0x20, 0x04, 0x10, 0x08, 0x08, 0x10, 0x04, 0x20, 0x02, 0x40, 0x01, 0x80, 0x01, 0x80, 0x02, 0x40, 0x04, 0x20, 0x08, 0x10, 0x10, 0x08, 0x20, 0x04, 0x40, 0x02, 0x80, 0x01};

void CProfile::glCommands(bool select, bool marked, bool no_color)
{
	if(marked && !no_color)
	{
		// show the sketches as highlighted
		for(std::list<int>::iterator It = m_sketches.begin(); It != m_sketches.end(); It++)
		{
			int sketch = *It;
			HeeksObj* object = heeksCAD->GetIDObject(SketchType, sketch);
			if(object)object->glCommands(false, true, false);
		}

		if(m_sketches.size() == 1)
		{
			// draw roll on point
			if(!m_params.m_auto_roll_on)
			{
				glColor3ub(0, 200, 200);
				glRasterPos3dv(m_params.m_roll_on_point);
				glBitmap(16, 16, 8, 8, 10.0, 0.0, cross16);
			}
			// draw roll off point
			if(!m_params.m_auto_roll_on)
			{
				glColor3ub(255, 128, 0);
				glRasterPos3dv(m_params.m_roll_off_point);
				glBitmap(16, 16, 8, 8, 10.0, 0.0, cross16);
			}
			// draw start point
			if(m_params.m_start_given)
			{
				glColor3ub(128, 0, 255);
				glRasterPos3dv(m_params.m_start);
				glBitmap(16, 16, 8, 8, 10.0, 0.0, cross16);
			}
			// draw end point
			if(m_params.m_end_given)
			{
				glColor3ub(200, 200, 0);
				glRasterPos3dv(m_params.m_end);
				glBitmap(16, 16, 8, 8, 10.0, 0.0, cross16);
			}
		}
	}
}

void CProfile::GetProperties(std::list<Property *> *list)
{
	m_params.GetProperties(this, list);
	HeeksObj::GetProperties(list);
}

static CProfile* object_for_pick = NULL;

class PickStart: public Tool{
	// Tool's virtual functions
	const wxChar* GetTitle(){return _("Pick Start");}
	void Run(){if(heeksCAD->PickPosition(_("Pick new start point"), object_for_pick->m_params.m_start))object_for_pick->m_params.m_start_given = true;}
	wxString BitmapPath(){ return _T("pickstart");}
};

static PickStart pick_start;

class PickEnd: public Tool{
	// Tool's virtual functions
	const wxChar* GetTitle(){return _("Pick End");}
	void Run(){if(heeksCAD->PickPosition(_("Pick new end point"), object_for_pick->m_params.m_end))object_for_pick->m_params.m_end_given = true;}
	wxString BitmapPath(){ return _T("pickend");}
};

static PickEnd pick_end;

void CProfile::GetTools(std::list<Tool*>* t_list, const wxPoint* p)
{
	object_for_pick = this;
	t_list->push_back(&pick_start);
	t_list->push_back(&pick_end);

	HeeksObj::GetTools(t_list, p);
}

HeeksObj *CProfile::MakeACopy(void)const
{
	return new CProfile(*this);
}

void CProfile::CopyFrom(const HeeksObj* object)
{
	operator=(*((CProfile*)object));
}

bool CProfile::CanAddTo(HeeksObj* owner)
{
	return owner->GetType() == OperationsType;
}

void CProfile::WriteXML(TiXmlNode *root)
{
	TiXmlElement * element = new TiXmlElement( "Profile" );
	root->LinkEndChild( element );  
	m_params.WriteXMLAttributes(element);

	// write sketch ids
	for(std::list<int>::iterator It = m_sketches.begin(); It != m_sketches.end(); It++)
	{
		int sketch = *It;
		TiXmlElement * sketch_element = new TiXmlElement( "sketch" );
		element->LinkEndChild( sketch_element );  
		sketch_element->SetAttribute("id", sketch);
	}

	WriteBaseXML(element);
}

// static member function
HeeksObj* CProfile::ReadFromXMLElement(TiXmlElement* element)
{
	CProfile* new_object = new CProfile;

	// read sketch ids
	for(TiXmlElement* pElem = TiXmlHandle(element).FirstChildElement().Element(); pElem; pElem = pElem->NextSiblingElement())
	{
		std::string name(pElem->Value());
		if(name == "params"){
			new_object->m_params.ReadFromXMLElement(pElem);
		}
		else if(name == "sketch"){
			for(TiXmlAttribute* a = pElem->FirstAttribute(); a; a = a->Next())
			{
				std::string name(a->Name());
				if(name == "id"){
					int id = a->IntValue();
					new_object->m_sketches.push_back(id);
				}
			}
		}
	}

	new_object->ReadBaseXML(element);

	return new_object;
}