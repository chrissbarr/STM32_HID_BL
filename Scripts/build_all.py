# !/usr/bin/python

from pathlib import Path
import subprocess

# list of build configs
configs = ["Debug", "Release"]

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
    print(f"Processing variant: {variant['name']}")

    for config in configs:
        print(f"Processing config: {config}")

        d = subprocess.run(["cmake", "..", "--preset", config, "-D", f"VARIANT={variant['name']}"], cwd='build')
        if (d.returncode is not 0):
            print("Detected error, aborting variant!")
            variant['pass'] = False
            break

        d = subprocess.run(["cmake", "--build", config], cwd='build')
        if (d.returncode is not 0):
            print("Detected error, aborting variant!")
            variant['pass'] = False
            break

passed = 0
for variant in variants_dict:
    if variant['pass']:
        passed += 1

print("----")
print(f"{passed} of {len(variants_dict)} builds succeeded.")
print("----")