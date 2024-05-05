# !/usr/bin/python

from pathlib import Path
import shutil
import subprocess
import sys

# list of build configs
configs = ["Release"]

# output dir for built variants
outputDir = Path('Binaries')
# output file extensions to put in output dir
outputFileFilter = ['.bin', '.hex', '.elf', '.map']

# remove existing built binaries
shutil.rmtree(outputDir)

# get list of all variants
p = Path('Variants')
variants = [x.name for x in p.iterdir() if x.is_dir()]
print(f"Found variants: {variants}")

variants_dict = []
for v in variants:
    vd = {}
    vd['name'] = v
    vd['pass'] = True
    variants_dict.append(vd)

# build each variant in each config
for variant in variants_dict:
    for config in configs:

        cfgName = f"{variant['name']}_{config}"
        print("----")
        print("----")
        print(f"== Processing: {cfgName} ==")

        print("== Configure ==")
        d = subprocess.run(["cmake", "..", "--preset", config, "-D", f"VARIANT={variant['name']}"], cwd='build')
        if (d.returncode != 0):
            print("Detected error, aborting variant!")
            variant['pass'] = False
            break

        print("== Build ==")
        d = subprocess.run(["cmake", "--build", config], cwd='build')
        if (d.returncode != 0):
            print("Detected error, aborting variant!")
            variant['pass'] = False
            break

        # create output directory if doesn't exist
        configOutputDir = outputDir / variant['name'] / config
        configOutputDir.mkdir(parents=True, exist_ok=True)

        buildDir = Path('build') / config
        files = [x for x in buildDir.glob('*') if x.is_file()]
        for file in files:
            if file.suffix in outputFileFilter:
                shutil.copy(file, configOutputDir)

        print(f"{cfgName} completed successfully!")

passed = 0
for variant in variants_dict:
    if variant['pass']:
        passed += 1

print("----")
print(f"{passed} of {len(variants_dict)} builds succeeded.")
print("----")