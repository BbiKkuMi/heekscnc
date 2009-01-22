// PythonStuff.cpp

#ifdef WIN32
#include "windows.h"
#endif

#include "PythonStuff.h"
#include "geometry/geometry.h"
#include <set>
#if _DEBUG
#undef _DEBUG
#include <Python.h>
#define _DEBUG
#else
#include <Python.h>
#endif


#ifdef WIN32

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		break;

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;

	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

#endif

 namespace geoff_geometry{

      int   UNITS = MM;

      double TOLERANCE = 1.0e-06;

      double TOLERANCE_SQ = TOLERANCE * TOLERANCE;

      double TIGHT_TOLERANCE = 1.0e-09;

      double UNIT_VECTOR_TOLERANCE = 1.0e-10;

      double RESOLUTION = 1.0e-06;

}

// dummy functions
namespace p4c {
const wchar_t* getMessage(const wchar_t* original, int messageGroup, int stringID){return original;}
const wchar_t* getMessage(const wchar_t* original){return original;}
void FAILURE(const wchar_t* str){throw(str);}
void FAILURE(const std::wstring& str){throw(str);}
}

std::set<Kurve*> valid_kurves;

static PyObject* kurve_new(PyObject* self, PyObject* args)
{
	Kurve* new_object = new Kurve();
	valid_kurves.insert(new_object);

	// return new object cast to an int
	PyObject *pValue = PyInt_FromLong((int)new_object);
	Py_INCREF(pValue);
	return pValue;
}

static PyObject* kurve_exists(PyObject* self, PyObject* args)
{
	int ik;
	if (!PyArg_ParseTuple(args, "i", &ik)) return NULL;

	Kurve* k = (Kurve*)ik;
	bool exists = (valid_kurves.find(k) != valid_kurves.end());

	// return exists
	PyObject *pValue = exists ? Py_True : Py_False;
	Py_INCREF(pValue);
	return pValue;
}

static PyObject* kurve_delete(PyObject* self, PyObject* args)
{
	int ik;
	if (!PyArg_ParseTuple(args, "i", &ik)) return NULL;

	Kurve* k = (Kurve*)ik;
	if(valid_kurves.find(k) != valid_kurves.end())
	{
		delete k;
		valid_kurves.erase(k);
	}

	Py_RETURN_NONE;
}

static PyObject* kurve_add_point(PyObject* self, PyObject* args)
{
	double x, y, i, j;
	int sp, ik;
	if (!PyArg_ParseTuple(args, "iidddd", &ik, &sp, &x, &y, &i, &j)) return NULL;

	Kurve* k = (Kurve*)ik;
	if(valid_kurves.find(k) != valid_kurves.end())
	{
		spVertex spv;
		spv.type = sp;
		spv.spanid = 0;
		spv.p.x = x;
		spv.p.y = y;
		spv.pc.x = i;
		spv.pc.y = j;
		if(sp)
		{
			// can't add arc as first span
			if(!(k->Started())){ const char* str = "can't add arc to kurve as first point"; cout << str; throw(str);}

			// fix radius by moving centre point a little bit
			int previous_vertex = k->nSpans();
			spVertex pv;
			k->Get(previous_vertex, pv);
			Vector2d v(spv.pc, pv.p);
			double r = v.magnitude();
			Circle c1( pv.p, r );
			Circle c2( spv.p, r );
			Point leftInters, rightInters;
			if(c1.Intof(c2, leftInters, rightInters) == 2)
			{
				double d1 = spv.pc.Dist(leftInters);
				double d2 = spv.pc.Dist(rightInters);
				if(d1<d2)spv.pc = leftInters;
				else spv.pc = rightInters;
			}
		}
		k->Add(spv);
	}

	Py_RETURN_NONE;
}

static PyObject* kurve_offset(PyObject* self, PyObject* args)
{
	int ik;
	int ik2;
	double left;
	if (!PyArg_ParseTuple(args, "iid", &ik, &ik2, &left)) return NULL;

	Kurve* k = (Kurve*)ik;
	Kurve* k2 = (Kurve*)ik2;
	int ret = 0;

	if(valid_kurves.find(k) != valid_kurves.end())
	{
		std::vector<Kurve*> offset_kurves;
		try
		{
			k->Offset(offset_kurves, fabs(left), (left > 0) ? 1:-1, 0, ret);
		}
		catch(const wchar_t* str)
		{
			wprintf(L"%s", str);
		}

		if(valid_kurves.find(k2) != valid_kurves.end())
		{
			if(offset_kurves.size() > 0)
			{
				*k2 = *(offset_kurves.front());
				ret = 1;
			}
		}
	}

	// return success
	PyObject *pValue = (ret != 0) ? Py_True : Py_False;
	Py_INCREF(pValue);
	return pValue;
}

static PyObject* kurve_copy(PyObject* self, PyObject* args)
{
	int ik;
	int ik2;
	if (!PyArg_ParseTuple(args, "ii", &ik, &ik2)) return NULL;
	Kurve* k = (Kurve*)ik;
	Kurve* k2 = (Kurve*)ik2;
	if(valid_kurves.find(k) != valid_kurves.end())
	{
		if(valid_kurves.find(k2) != valid_kurves.end())
		{
			*k2 = *k;
		}
	}

	Py_RETURN_NONE;
}

static PyObject* kurve_num_spans(PyObject* self, PyObject* args)
{
	int ik;
	if (!PyArg_ParseTuple(args, "i", &ik)) return NULL;
	Kurve* k = (Kurve*)ik;

	int n = 0;
	if(valid_kurves.find(k) != valid_kurves.end())
	{
		n = k->nSpans();
	}

	PyObject *pValue = PyInt_FromLong(n);
	Py_INCREF(pValue);
	return pValue;
}

static PyObject* kurve_get_span(PyObject* self, PyObject* args)
{
	int ik;
	int index; // 0 is first
	if (!PyArg_ParseTuple(args, "ii", &ik, &index)) return NULL;
	Kurve* k = (Kurve*)ik;

	int sp = -1;
	double sx = 0.0, sy = 0.0;
	double ex = 0.0, ey = 0.0;
	double cx = 0.0, cy = 0.0;

	if(valid_kurves.find(k) != valid_kurves.end())
	{
		int n = k->nSpans();
		if(index >=0 && index <n)
		{
			Span span;
			k->Get(index + 1, span);
			sp = span.dir;
			sx = span.p0.x;
			sy = span.p0.y;
			ex = span.p1.x;
			ey = span.p1.y;
			cx = span.pc.x;
			cy = span.pc.y;
		}
	}

	// return span data as a tuple
	PyObject *pTuple = PyTuple_New(7);
	{
		PyObject *pValue = PyInt_FromLong(sp);
		if (!pValue){
			Py_DECREF(pTuple);return NULL;
		}
		PyTuple_SetItem(pTuple, 0, pValue);
	}
	{
		PyObject *pValue = PyFloat_FromDouble(sx);
		if (!pValue){
			Py_DECREF(pTuple);return NULL;
		}
		PyTuple_SetItem(pTuple, 1, pValue);
	}
	{
		PyObject *pValue = PyFloat_FromDouble(sy);
		if (!pValue){
			Py_DECREF(pTuple);return NULL;
		}
		PyTuple_SetItem(pTuple, 2, pValue);
	}
	{
		PyObject *pValue = PyFloat_FromDouble(ex);
		if (!pValue){
			Py_DECREF(pTuple);return NULL;
		}
		PyTuple_SetItem(pTuple, 3, pValue);
	}
	{
		PyObject *pValue = PyFloat_FromDouble(ey);
		if (!pValue){
			Py_DECREF(pTuple);return NULL;
		}
		PyTuple_SetItem(pTuple, 4, pValue);
	}
	{
		PyObject *pValue = PyFloat_FromDouble(cx);
		if (!pValue){
			Py_DECREF(pTuple);return NULL;
		}
		PyTuple_SetItem(pTuple, 5, pValue);
	}
	{
		PyObject *pValue = PyFloat_FromDouble(cy);
		if (!pValue){
			Py_DECREF(pTuple);return NULL;
		}
		PyTuple_SetItem(pTuple, 6, pValue);
	}

	Py_INCREF(pTuple);
	return pTuple;
}

static PyObject* kurve_get_span_dir(PyObject *self, PyObject *args)
{
	int ik;
	int index; // 0 is first
	double fraction;
	if (!PyArg_ParseTuple(args, "iid", &ik, &index, &fraction)) return NULL;
	Kurve* k = (Kurve*)ik;

	double vx = 1.0;
	double vy = 0.0;

	if(valid_kurves.find(k) != valid_kurves.end())
	{
		int n = k->nSpans();
		if(index >=0 && index <n)
		{
			Span span;
			k->Get(index + 1, span);
			Vector2d v = span.GetVector(fraction);
			vx = v.getx();
			vy = v.gety();
		}
	}

	// return span data as a tuple
	PyObject *pTuple = PyTuple_New(2);
	{
		PyObject *pValue = PyFloat_FromDouble(vx);
		if (!pValue){
			Py_DECREF(pTuple); return NULL;
		}
		PyTuple_SetItem(pTuple, 0, pValue);
	}
	{
		PyObject *pValue = PyFloat_FromDouble(vy);
		if (!pValue){
			Py_DECREF(pTuple); return NULL;
		}
		PyTuple_SetItem(pTuple, 1, pValue);
	}

	Py_INCREF(pTuple);
	return pTuple;
}

void tangential_arc(const Point &p0, const Point &p1, const Vector2d &v0, Point &c, int &dir)
{
	// sets dir to 0, if a line is needed, else to 1 or -1 for acw or cw arc and sets c
	dir = 0;

	if(p0.Dist(p1) > 0.0000000001 && v0.magnitude() > 0.0000000001){
		Vector2d v1(p0, p1);
		Point halfway(p0 + Point(v1 * 0.5));
		Plane pl1(halfway, v1);
		Plane pl2(p0, v0);
		Line plane_line;
		if(pl1.Intof(pl2, plane_line))
		{
			Line l1(halfway, v1);
			double t1, t2;
			Line lshort;
			plane_line.Shortest(l1, lshort, t1, t2);
			c = lshort.p0;
			Vector3d cross = Vector3d(v0) ^ Vector3d(v1);
			dir = (cross.getz() > 0) ? 1:-1;
		}
	}
}

static PyObject* kurve_tangential_arc(PyObject* self, PyObject* args)
{
//                 rcx, rcy, rdir = tangential_arc(sx, sy, svx, svy, ex, ey)
	double sx, sy, ex, ey, vx, vy;
	if (!PyArg_ParseTuple(args, "dddddd", &sx, &sy, &vx, &vy, &ex, &ey)) return NULL;

	Point p0(sx, sy);
	Point p1(ex, ey);
	Vector2d v0(vx, vy);
	Point c(0, 0);
	int dir = 0;

	tangential_arc(p0, p1, v0, c, dir);

	// return span data as a tuple
	PyObject *pTuple = PyTuple_New(3);
	{
		PyObject *pValue = PyFloat_FromDouble(c.x);
		if (!pValue){
			Py_DECREF(pTuple);return NULL;
		}
		PyTuple_SetItem(pTuple, 0, pValue);
	}
	{
		PyObject *pValue = PyFloat_FromDouble(c.y);
		if (!pValue){
			Py_DECREF(pTuple);return NULL;
		}
		PyTuple_SetItem(pTuple, 1, pValue);
	}
	{
		PyObject *pValue = PyInt_FromLong(dir);
		if (!pValue){
			Py_DECREF(pTuple);return NULL;
		}
		PyTuple_SetItem(pTuple, 2, pValue);
	}

	Py_INCREF(pTuple);
	return pTuple;
}

static PyMethodDef KurveMethods[] = {
	{"new", kurve_new, METH_VARARGS , ""},
	{"exists", kurve_exists, METH_VARARGS , ""},
	{"delete", kurve_delete, METH_VARARGS , ""},
	{"add_point", kurve_add_point, METH_VARARGS , ""},
	{"offset", kurve_offset, METH_VARARGS , ""},
	{"copy", kurve_copy, METH_VARARGS , ""},
	{"num_spans", kurve_num_spans, METH_VARARGS , ""},
	{"get_span", kurve_get_span, METH_VARARGS , ""},
	{"get_span_dir", kurve_get_span_dir, METH_VARARGS , ""},
	{"tangential_arc", kurve_tangential_arc, METH_VARARGS , ""},
	{NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC
initkurve(void)
{
	Py_InitModule("kurve", KurveMethods);
}
