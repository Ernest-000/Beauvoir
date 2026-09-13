import subprocess, os

BGS_PATH = "bin/bergson"
BGS_XML_DIR = "xml/"
BGS_FILES = []

def bgs_get_xml_files():
    for file in os.listdir(BGS_XML_DIR):
        if file.endswith(".ui"):
            BGS_FILES.append(os.path.join(BGS_XML_DIR, file))

if __name__ == "__main__":
    bgs_get_xml_files()
    for f in BGS_FILES:
        base, _ = os.path.splitext(f)
        nf = base + ".xml"
        
        print(f"converting {f} -> {nf}...")

        with open(nf, "w") as out_file:
            subprocess.run(
                ["gtk4-builder-tool", "simplify", "--3to4", f], 
                stdout=out_file
            )
        
        print(f"validating {nf}...")
        subprocess.run(["gtk4-builder-tool", "validate", nf])
        
        print(f"finished processing {f}\n")

    subprocess.run(BGS_PATH)