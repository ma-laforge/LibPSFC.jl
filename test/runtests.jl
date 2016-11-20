#Test code
#-------------------------------------------------------------------------------

using LibPSFC

#No real test code yet... just demonstrate use:

function testfileaccess(path::String)
	println("\n\nfile: $path")
	println("------------------")
	reader = LibPSFC._open(path)
	display(reader.properties)
	@show names(reader)
	close(reader)
end

sampledata(filename::String) = joinpath(LibPSFC.rootpath, "core/data", filename)

testfileaccess(sampledata("opBegin"))
testfileaccess(sampledata("pss0.fd.pss"))
testfileaccess(sampledata("timeSweep"))
testfileaccess(sampledata("srcSweep"))

signame = "INN"
println("\nRead in $signame vector:")
println("------------------")
reader = LibPSFC._open(sampledata("timeSweep"))
t = readsweep(reader)
y = read(reader, signame)
close(reader)

@show t[1], t[end]
flush(STDOUT)

:Test_Complete
