#LibPSF: A Julia wrapper for libpsf tools
#-------------------------------------------------------------------------------
module LibPSF

const rootpath = realpath(joinpath(dirname(realpath(@__FILE__)),"../."))
const libroot = joinpath(rootpath, "core/lib")
const objfile = joinpath(libroot, "libpsfreader_c.so")

include("base.jl")
#include("show.jl")

export readsweep, readscalar

end #LibPSF
#Last line
