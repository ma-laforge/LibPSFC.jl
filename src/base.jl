#Julia test


#==Constants
===============================================================================#
module PSFResult
	const Success = 0

	#Exceptions:
	const NotImplemented = 1
	const FileOpenError = 2
	const InvalidFileError = 3
	const FileCloseError = 4
	const NotFound = 5
	const DataSetNotOpen = 6
	const PropertyNotFound = 7
	const Unknown = 8
end #module PSFResult

module PSFElemType
	const E_UNKNOWN = 0
	const E_INT8 = 1
	const E_INT32 = 2
	const E_DOUBLE = 3
	const E_COMPLEXDOUBLE = 4
	const E_STRING = 5
	const E_STRUCT = 6
end #module PSFElemType


#==Main types
===============================================================================#
typealias PropDict Dict{String, Any}

type PSFDataSetRef
	ref::Ptr{Void}
end
PSFDataSetRef() = PSFDataSetRef(0)

type DataReader
	ds::PSFDataSetRef
	filepath::String
	properties::PropDict
	nsig::Int #Not currently used
	npts::Int #Not currently used
	strbuf1::Vector{UInt8} #For reading
	strbuf2::Vector{UInt8} #For reading
end
function DataReader(filepath::String)
	result = DataReader(PSFDataSetRef(), filepath, PropDict(), 0, 0, Array(UInt8, 1000), Array(UInt8, 1000))
	result.strbuf1[1] = 0
	result.strbuf2[1] = 0
	return result
end


#==Helper functions
===============================================================================#
function raiseonerror(returnval::Cint)
	if PSFResult.Success == returnval
		return
	elseif PSFResult.NotImplemented == returnval
		error("Not implemented")
	elseif PSFResult.FileOpenError == returnval
		error("File open error")
	elseif PSFResult.InvalidFileError == returnval
		error("Invalid file error")
	elseif PSFResult.FileCloseError == returnval
		error("File close error")
	elseif PSFResult.NotFound == returnval
		error("Not found")
	elseif PSFResult.DataSetNotOpen == returnval
		error("Dataset not open")
	elseif PSFResult.PropertyNotFound == returnval
		error("Property not found")
	else
		error("Unknown error: $returnval")
	end
end

function getpsfelemtype(elemtypeid::Cint)
	if PSFElemType.E_INT8 == elemtypeid
		return Int8;
	elseif PSFElemType.E_INT32 == elemtypeid
		return Int32;
	elseif PSFElemType.E_DOUBLE == elemtypeid
		return Cdouble;
	elseif PSFElemType.E_COMPLEXDOUBLE == elemtypeid
		return Complex{Cdouble};
#	elseif PSFElemType.E_STRING == elemtypeid
#		return Cstring;
#	elseif PSFElemType.E_STRUCT == elemtypeid
#		return ??;
	else
		error("PSF element type not recognized: $elemtypeid.")
	end
end

#readpsfscalar: Read in buffered scalar value.
readpsfscalar{T}(reader::DataReader, ::Type{T}) = error("PSF element type not supported $T.")
readpsfscalar(reader::DataReader, ::Type{Int8}) =
	ccall((:PSF_GetScalarInt8, objfile), Int8, (Ptr{Void},), reader.ds.ref)
readpsfscalar(reader::DataReader, ::Type{Int32}) =
	ccall((:PSF_GetScalarInt32, objfile), Int32, (Ptr{Void},), reader.ds.ref)
readpsfscalar(reader::DataReader, ::Type{Cdouble}) =
	ccall((:PSF_GetScalarDouble, objfile), Cdouble, (Ptr{Void},), reader.ds.ref)
readpsfscalar(reader::DataReader, ::Type{Complex{Cdouble}}) =
	ccall((:PSF_GetScalarCplxDouble, objfile), Complex{Cdouble}, (Ptr{Void},), reader.ds.ref)

#readpsfvec assuption: v is sized large enough to read in entire vector
readpsfvec{T}(reader::DataReader, v::Vector{T}) = error("PSF element type not supported $T.")
readpsfvec(reader::DataReader, v::Vector{Int8}) =
	raiseonerror(ccall((:PSF_CopySigInt8, objfile), Cint, (Ptr{Void}, Ptr{Int8}),
		reader.ds.ref, v))
readpsfvec(reader::DataReader, v::Vector{Int32}) =
	raiseonerror(ccall((:PSF_CopySigInt32, objfile), Cint, (Ptr{Void}, Ptr{Int32}),
		reader.ds.ref, v))
readpsfvec(reader::DataReader, v::Vector{Cdouble}) =
	raiseonerror(ccall((:PSF_CopySigDouble, objfile), Cint, (Ptr{Void}, Ptr{Cdouble}),
		reader.ds.ref, v))
#==Question:
Can we guarantee that all c++ implementations of std::complex<double> uses the same
real/imag ordering as Julia's Complex{Cdouble}?==#
readpsfvec(reader::DataReader, v::Vector{Complex{Cdouble}}) =
	raiseonerror(ccall((:PSF_CopySigComplexDouble, objfile), Cint, (Ptr{Void}, Ptr{Complex{Cdouble}}),
		reader.ds.ref, v))


#==Misc readers
===============================================================================#

function readproperties(reader::DataReader)
	result = PropDict()

	if ccall((:PSFHeader_GetPropFirst, objfile), Cint,
		(Ptr{Void}, Ptr{UInt8}, Cint, Ptr{UInt8}, Cint),
		reader.ds.ref, reader.strbuf1, length(reader.strbuf1), reader.strbuf2, length(reader.strbuf2)
	) != PSFResult.Success; return result; end
	push!(result, unsafe_string(pointer(reader.strbuf1)) => unsafe_string(pointer(reader.strbuf2)))

	while PSFResult.Success == ccall((:PSFHeader_GetPropNext, objfile), Cint,
		(Ptr{Void}, Ptr{UInt8}, Cint, Ptr{UInt8}, Cint),
		reader.ds.ref, reader.strbuf1, length(reader.strbuf1), reader.strbuf2, length(reader.strbuf2))

		push!(result, unsafe_string(pointer(reader.strbuf1)) => unsafe_string(pointer(reader.strbuf2)))
	end

	return result
end

function Base.names(reader::DataReader)
	result = String[]

	if ccall((:PSF_GetSigNamesFirst, objfile), Cint, (Ptr{Void}, Ptr{UInt8}, Cint),
		reader.ds.ref, reader.strbuf1, length(reader.strbuf1)
	) != PSFResult.Success; return result; end
	push!(result, unsafe_string(pointer(reader.strbuf1)))

	while PSFResult.Success == ccall((:PSF_GetSigNamesNext, objfile), Cint,
		(Ptr{Void}, Ptr{UInt8}, Cint), reader.ds.ref, reader.strbuf1, length(reader.strbuf1))

		push!(result, unsafe_string(pointer(reader.strbuf1)))
	end

	return result
end


#==Open/close/read functions
===============================================================================#
function Base.open(::Type{PSFDataSetRef}, filepath::String)
	result = PSFDataSetRef()
	raiseonerror(ccall((:PSFDataSet_Open, objfile), Cint, (Cstring, Ptr{PSFDataSetRef}),
		filepath, pointer_from_objref(result)
	))
	finalizer(result, close)
	return result
end
function Base.open(::Type{DataReader}, filepath::String)
	result = DataReader(filepath)
	result.ds = open(PSFDataSetRef, result.filepath)
	result.properties = readproperties(result)
	return result
end
_open(filepath::String) = open(DataReader, filepath)

function Base.close(ds::PSFDataSetRef)
	raiseonerror(ccall((:PSFDataSet_Close, objfile), Cint, (Ptr{Void},),
		ds.ref
	))
	ds.ref = 0
end

Base.close(reader::DataReader) = close(reader.ds)

#Read in vector buffered by C++ reader:
function _readbufferedvec(reader::DataReader)
	siglen = ccall((:PSF_GetSigLen, objfile), Cint, (Ptr{Void},),
		reader.ds.ref
	)
	elemtypeid = ccall((:PSF_GetSigType, objfile), Cint, (Ptr{Void},),
		reader.ds.ref
	)
	v = Array(getpsfelemtype(elemtypeid), siglen)
	readpsfvec(reader, v)
	return v
end

#Read in sweep vector:
function readsweep(reader::DataReader)
	raiseonerror(ccall((:PSF_ReadSigSweep, objfile), Cint, (Ptr{Void},),
		reader.ds.ref
	))
	return _readbufferedvec(reader)
end

#Read in a signal by name:
function Base.read(reader::DataReader, signame::String)
	raiseonerror(ccall((:PSF_ReadSig, objfile), Cint, (Ptr{Void}, Cstring),
		reader.ds.ref, signame
	))
	return _readbufferedvec(reader)
end

#Read in a scalar value by name:
function readscalar(reader::DataReader, signame::String)
	raiseonerror(ccall((:PSF_ReadScalar, objfile), Cint, (Ptr{Void}, Cstring),
		reader.ds.ref, signame
	))
	elemtypeid = ccall((:PSF_GetScalarType, objfile), Cint, (Ptr{Void},),
		reader.ds.ref
	)
	return readpsfscalar(reader, getpsfelemtype(elemtypeid))
end

#Last line
