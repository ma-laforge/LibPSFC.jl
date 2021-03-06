/* C-interface to libpsf tool.

Created to streamline Julia integration.
This interface is not pretty.
*/
#include <stdio.h>
#include <cstring>
#include <string>

#include "psf.h"
#include "psfdata.h"


//Constants
//******************************************************************************

enum PSFResult {
	PSF_Success = 0,
	//Exceptions:
	PSF_NotImplemented,
	PSF_FileOpenError,
	PSF_InvalidFileError,
	PSF_FileCloseError,
	PSF_NotFound,
	PSF_DataSetNotOpen,
	PSF_PropertyNotFound,
	PSF_Unknown,
};

enum PSFElemType {
	PSFELEM_UNKNOWN = 0,
	PSFELEM_INT8,
	PSFELEM_INT32,
	PSFELEM_DOUBLE,
	PSFELEM_COMPLEXDOUBLE,
	PSFELEM_STRING,
	PSFELEM_STRUCT,
};


//Helper objects
//******************************************************************************

//Object used to hold state while accessing PSFDataset with the C-interface:
class PSFDataSet_C {
public:
	PSFDataSet ds;
	PropertyMap pmap;
	PropertyMap::iterator pmap_iter;
	std::vector<std::string> strvec;
	size_t strvec_pos;
	PSFVector *sigvec;

	//Not sure how to copy/release PSFScalar objects (as done with PSFVector).
	//Could not find code on how to read PSFScalar.
	//But get_signal_scalar() returns a const PSFScalar&
	//Guess: release is not necessary, but lifetime might be limited to calling scope.
	//Solution: Figure out type when read, and store here until user reads:
	PSFElemType scalar_type;
	PSFInt8 scalar_int8;
	PSFInt32 scalar_int32;
	PSFDouble scalar_double;
	PSFComplexDouble scalar_cplxdouble;
//	PSFString scalar_string;

	PSFDataSet_C(std::string filename):
		ds(filename), strvec_pos(0), sigvec(0), scalar_type(PSFELEM_UNKNOWN) {};
	~PSFDataSet_C();

	PSFElemType BufferValue(const PSFScalar &s);
};

PSFDataSet_C::~PSFDataSet_C() {
	if(this->sigvec) delete this->sigvec;
};


//Helper functions
//******************************************************************************

//Convert errors to a PSFResult value:
PSFResult PSFGetError(std::exception &e) {
	if (dynamic_cast<NotImplemented*>(&e)) {
		return PSF_NotImplemented;
	} else if (dynamic_cast<FileOpenError*>(&e)) {
		return PSF_FileOpenError;
	} else if (dynamic_cast<InvalidFileError*>(&e)) {
		return PSF_InvalidFileError;
	} else if (dynamic_cast<FileCloseError*>(&e)) {
		return PSF_FileCloseError;
	} else if (dynamic_cast<NotFound*>(&e)) {
		return PSF_NotFound;
	} else if (dynamic_cast<DataSetNotOpen*>(&e)) {
		return PSF_DataSetNotOpen;
	} else if (dynamic_cast<PropertyNotFound*>(&e)) {
		return PSF_PropertyNotFound;
	}
	return PSF_Unknown;
}

//Find element type of a PSFScalar:
PSFElemType PSFGetScalarElemType(const PSFScalar &s) {
	if (dynamic_cast<const PSFInt8Scalar*>(&s)) {
		return PSFELEM_INT8;
	} else if (dynamic_cast<const PSFInt32Scalar*>(&s)) {
		return PSFELEM_INT32;
	} else if (dynamic_cast<const PSFDoubleScalar*>(&s)) {
		return PSFELEM_DOUBLE;
	} else if (dynamic_cast<const PSFComplexDoubleScalar*>(&s)) {
		return PSFELEM_COMPLEXDOUBLE;
	} else if (dynamic_cast<const PSFStringScalar*>(&s)) {
		return PSFELEM_STRING;
	} else if (dynamic_cast<const StructScalar*>(&s)) {
		return PSFELEM_STRUCT;
	}
	return PSFELEM_UNKNOWN;
}

//Find element type of a PSFVector:
PSFElemType PSFGetVectorElemType(PSFVector &v) {
	if (dynamic_cast<PSFInt8Vector*>(&v)) {
		return PSFELEM_INT8;
	} else if (dynamic_cast<PSFInt32Vector*>(&v)) {
		return PSFELEM_INT32;
	} else if (dynamic_cast<PSFDoubleVector*>(&v)) {
		return PSFELEM_DOUBLE;
	} else if (dynamic_cast<PSFComplexDoubleVector*>(&v)) {
		return PSFELEM_COMPLEXDOUBLE;
	} else if (dynamic_cast<PSFStringVector*>(&v)) {
		return PSFELEM_STRING;
	} else if (dynamic_cast<StructVector*>(&v)) {
		return PSFELEM_STRUCT;
	}
	return PSFELEM_UNKNOWN;
}

template<class T>
T PSFGetScalar(const PSFScalar &s) {
	const PSFScalarT<T> *p;
	p = dynamic_cast<const PSFScalarT<T>*>(&s);
	if (p) return p->value;
	return T(); //Generates 0-value if not of that type.
}

//Copy PSFScalar value to PSFDataSet_C registers:
//TODO: Inefficient.  Find better way.
PSFElemType PSFDataSet_C::BufferValue(const PSFScalar &s) {
	PSFElemType result;
	result = PSFGetScalarElemType(s);
	this->scalar_int8 = PSFGetScalar<PSFInt8>(s);
	this->scalar_int32 = PSFGetScalar<PSFInt32>(s);
	this->scalar_double = PSFGetScalar<PSFDouble>(s);
	this->scalar_cplxdouble = PSFGetScalar<PSFComplexDouble>(s);
//	this->scalar_string = PSFGetScalar<PSFString>(s);
	this->scalar_type = result;
	return result;
}


//PropertyMap tools
//******************************************************************************

int PSF_CopyPMap(const PropertyMap::iterator &iter, char *keybuf, int kblen, char *valbuf, int vblen) {
	std::string strbuf(iter->second->tostring());
	strncpy(keybuf, iter->first.c_str(), kblen);
	strncpy(valbuf, strbuf.c_str(), vblen);
	return PSF_Success;
}

int PSF_GetPropNext(PSFDataSet_C *dsc, char *keybuf, int kblen, char *valbuf, int vblen) {
	if (!dsc) return PSF_NotFound;
	++(dsc->pmap_iter);
	if (dsc->pmap_iter==dsc->pmap.end()) return PSF_PropertyNotFound;
	return PSF_CopyPMap(dsc->pmap_iter, keybuf, kblen, valbuf, vblen);
}

//Safe way to delete PSFDataSet_C::sigvec (also resets pointer to 0):
int PSF_DeleteSigVec(PSFDataSet_C *dsc) {
	if (!dsc) return PSF_NotFound;
	if(dsc->sigvec) delete dsc->sigvec;
	dsc->sigvec = 0;
	return PSF_Success;
}


//File properties
//******************************************************************************

extern "C"
int PSFHeader_GetPropFirst(PSFDataSet_C *dsc, char *keybuf, int kblen, char *valbuf, int vblen) {
	if (!dsc) return PSF_NotFound;
	dsc->pmap = dsc->ds.get_header_properties();
	dsc->pmap_iter = dsc->pmap.begin();
	if (dsc->pmap_iter==dsc->pmap.end()) return PSF_PropertyNotFound;
	return PSF_CopyPMap(dsc->pmap_iter, keybuf, kblen, valbuf, vblen);
}

extern "C"
int PSFHeader_GetPropNext(PSFDataSet_C *dsc, char *keybuf, int kblen, char *valbuf, int vblen) {
	return PSF_GetPropNext(dsc, keybuf, kblen, valbuf, vblen);
}


//Signal names
//******************************************************************************
extern "C"
int PSF_GetSigNamesFirst(PSFDataSet_C *dsc, char *namebuf, int nblen) {
	if (!dsc) return PSF_NotFound;
	dsc->strvec = dsc->ds.get_signal_names();
	dsc->strvec_pos = 0;
	if (dsc->strvec.size()<=dsc->strvec_pos) return PSF_PropertyNotFound;
	strncpy(namebuf, dsc->strvec[dsc->strvec_pos].c_str(), nblen);
	return PSF_Success;
}

extern "C"
int PSF_GetSigNamesNext(PSFDataSet_C *dsc, char *namebuf, int nblen) {
	if (!dsc) return PSF_NotFound;
	++dsc->strvec_pos;
	if (dsc->strvec.size()<=dsc->strvec_pos) return PSF_PropertyNotFound;
	strncpy(namebuf, dsc->strvec[dsc->strvec_pos].c_str(), nblen);
	return PSF_Success;
}


//Scalars
//******************************************************************************
extern "C"
int PSF_ReadScalar(PSFDataSet_C *dsc, char *signame) {
	if (!dsc) return PSF_NotFound;
	try {
		const PSFScalar &s = dsc->ds.get_signal_scalar(signame);
		dsc->BufferValue(s);
	}
	catch (std::exception &e) {return PSFGetError(e);}
	catch (...) {return PSF_Unknown;}

	return PSF_Success;
}

extern "C"
int PSF_GetScalarType(PSFDataSet_C *dsc) {
	if (!dsc) return PSFELEM_UNKNOWN;
	return dsc->scalar_type;
}

extern "C"
int8_t PSF_GetScalarInt8(PSFDataSet_C *dsc) {
	if (!dsc) return 0;
	return dsc->scalar_int8;
}
extern "C"
int32_t PSF_GetScalarInt32(PSFDataSet_C *dsc) {
	if (!dsc) return 0;
	return dsc->scalar_int32;
}
extern "C"
double PSF_GetScalarDouble(PSFDataSet_C *dsc) {
	if (!dsc) return 0;
	return dsc->scalar_double;
}
extern "C"
std::complex<double> PSF_GetScalarCplxDouble(PSFDataSet_C *dsc) {
	if (!dsc) return 0;
	return dsc->scalar_cplxdouble;
}


//Signals
//******************************************************************************
extern "C"
int PSF_ReadSigSweep(PSFDataSet_C *dsc) {
	if (!dsc) return PSF_NotFound;
	PSF_DeleteSigVec(dsc); //Delete previous

	try {
		dsc->sigvec = dsc->ds.get_sweep_values();
	}
	catch (std::exception &e) {return PSFGetError(e);}
	catch (...) {return PSF_Unknown;}

	return PSF_Success;
}

extern "C"
int PSF_ReadSig(PSFDataSet_C *dsc, char *signame) {
	if (!dsc) return PSF_NotFound;
	PSF_DeleteSigVec(dsc); //Delete previous

	try {
		dsc->sigvec = dsc->ds.get_signal_vector(signame);
	}
	catch (std::exception &e) {return PSFGetError(e);}
	catch (...) {return PSF_Unknown;}

	return PSF_Success;
}

extern "C"
unsigned long PSF_GetSigLen(PSFDataSet_C *dsc) {
	if (!dsc) return 0;
	if (!dsc->sigvec) return 0;
	return dsc->sigvec->size();
}

extern "C"
int PSF_GetSigType(PSFDataSet_C *dsc) {
	if (!dsc) return PSFELEM_UNKNOWN;
	if (!dsc->sigvec) return PSFELEM_UNKNOWN;
	return PSFGetVectorElemType(*dsc->sigvec);
}

template<class T>
int PSF_CopySig(PSFDataSet_C *dsc, T *dest) {
	std::vector<T> *src = 0;
	if (!dsc) return PSF_NotFound;
	if (!dsc->sigvec) return PSF_NotFound;
	src = dynamic_cast<std::vector<T>*>(dsc->sigvec);
	if (!src) return PSF_Unknown;
	for (size_t i=0; i < src->size(); ++i) {
		*(dest++) = (*src)[i];
	}
	return PSF_Success;
}

extern "C"
int PSF_CopySigInt8(PSFDataSet_C *dsc, int8_t *dest) {
	return PSF_CopySig<PSFInt8>(dsc, dest);
}
extern "C"
int PSF_CopySigInt32(PSFDataSet_C *dsc, int32_t *dest) {
	return PSF_CopySig<PSFInt32>(dsc, dest);
}
extern "C"
int PSF_CopySigDouble(PSFDataSet_C *dsc, double *dest) {
	return PSF_CopySig<PSFDouble>(dsc, dest);
}
extern "C"
int PSF_CopySigComplexDouble(PSFDataSet_C *dsc, std::complex<double> *dest) {
	return PSF_CopySig<PSFComplexDouble>(dsc, dest);
}


//File open/read/close
//******************************************************************************

extern "C"
int PSFDataSet_Open(const char *filename, PSFDataSet_C **newobj) {
	*newobj = 0;

	try {
		std::string filestr(filename);
		*newobj = new PSFDataSet_C(filestr);
	}
	catch (std::exception &e) {return PSFGetError(e);}
	catch (...) {return PSF_Unknown;}

	return PSF_Success;
}

extern "C"
int PSFDataSet_Close(PSFDataSet_C *dsc) {
	if (dsc) delete dsc;
	return PSF_Success;
}

//Last line
