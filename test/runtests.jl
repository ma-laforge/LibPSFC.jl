#Test code
#-------------------------------------------------------------------------------

using LibPSF

#No real test code yet... just demonstrate use:

function testfileaccess(path::AbstractString)
	println("\n\nfile: $path")
	println("------------------")
	reader = LibPSF._open(path)
	display(reader.prop)
	@show names(reader)
	close(reader)
end

sampledata(filename::AbstractString) = joinpath(LibPSF.rootpath, "core/data", filename)

testfileaccess(sampledata("opBegin"))
testfileaccess(sampledata("pss0.fd.pss"))
testfileaccess(sampledata("timeSweep"))
testfileaccess(sampledata("srcSweep"))

signame = "INN"
println("\nRead in $signame vector:")
println("------------------")
reader = LibPSF._open(sampledata("timeSweep"))
t = readsweep(reader)
y = read(reader, signame)
close(reader)

@show t[1], t[end]
flush(STDOUT)

:Test_Complete
