// Ghidra headless script: write the decompiled C of every function to a file.
// Usage (analyzeHeadless): -postScript DecompileAll.java <out.c>
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.listing.*;
import java.io.*;

public class DecompileAll extends GhidraScript {
    @Override
    public void run() throws Exception {
        String out = getScriptArgs()[0];
        DecompInterface di = new DecompInterface();
        di.openProgram(currentProgram);
        try (PrintWriter w = new PrintWriter(new FileWriter(out))) {
            for (Function f : currentProgram.getFunctionManager().getFunctions(true)) {
                DecompileResults r = di.decompileFunction(f, 120, monitor);
                w.println("// ==== " + f.getName() + " @ " + f.getEntryPoint() +
                          " size " + f.getBody().getNumAddresses());
                w.println(r.decompileCompleted() ? r.getDecompiledFunction().getC()
                                                 : "// decompile failed: " + r.getErrorMessage());
            }
        }
        di.dispose();
    }
}
