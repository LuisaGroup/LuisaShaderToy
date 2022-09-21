from sys import argv
from os import listdir, system
from os.path import dirname, abspath


if __name__ == "__main__":
    exe_dir = argv[1]
    script_dir = f"{dirname(abspath(__file__))}"
    apps = [f[:-4] for f in listdir(f"{script_dir}/../src/apps") if f.endswith(".cpp")]
    print(apps)
    for app in apps:
        print(f"Recording {app}")
        system(f"{exe_dir}/{app} -o {script_dir}/{app}.mp4 -t 0.033333 --size 1280x720 --fps 30 -n 300 -b metal")
