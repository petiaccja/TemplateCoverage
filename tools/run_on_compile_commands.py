import pathlib
import json
import shlex
import re
import argparse
import copy
import os
import subprocess
import tempfile
from typing import Optional


def is_modmap_arg(arg: str) -> bool:
    return re.match(r"\@.*\.modmap", arg)


def get_include_arg(arg: str) -> Optional[str]:
    if re.match(r"-(i|I).*", arg):
        return arg[2:]
    return None


def clean_compile_commands_entry(cc_entry: dict) -> dict:
    args = cc_entry["arguments"]
    args = [arg for arg in args if not is_modmap_arg(arg)]
    new = copy.copy(cc_entry)
    new["arguments"] = args
    return new


def clean_compile_commands(compile_commands: list) -> list:
    return [clean_compile_commands_entry(entry) for entry in compile_commands]


def replace_verify_headers_entry(cc_entry: dict) -> dict:
    file_str = cc_entry["file"]
    file = pathlib.Path(file_str)
    verify_parts = [i for i, part in enumerate(file.parts) if re.match(r".*_verify_interface_header_sets", part)]
    if verify_parts:
        root_idx = verify_parts[-1]
        root_dir = pathlib.Path(file.parts[0]).joinpath(*file.parts[1:root_idx + 1])
        header_path = file.relative_to(root_dir).with_name(file.stem)

        all_args = cc_entry["arguments"]
        maybe_include_dirs = [get_include_arg(arg) for arg in all_args]
        include_dirs = [pathlib.Path(include_dir) for include_dir in maybe_include_dirs if include_dir is not None]
        matched_include_dirs = [include_dir for include_dir in include_dirs if (include_dir / header_path).exists()]

        if matched_include_dirs:
            new_file = matched_include_dirs[0] / header_path
            new_args = [str(new_file) if arg == file_str else arg for arg in all_args if not any(arg.endswith(str(idir)) for idir in matched_include_dirs)]
            new_args = [arg for arg in new_args]
            new_entry = copy.copy(cc_entry)
            new_entry["file"] = str(new_file)
            new_entry["arguments"] = [*new_args, "-Wno-pragma-once-outside-header"]
            return new_entry
        else:
            return cc_entry
    else:
        return cc_entry


def replace_verify_headers(compile_commands: list) -> list:
    return [replace_verify_headers_entry(entry) for entry in compile_commands]


def process_compile_commands(compile_commands: list) -> list:
    compile_commands = replace_verify_headers(compile_commands)
    compile_commands = clean_compile_commands(compile_commands)
    return compile_commands


def command_to_args(cc_entry: dict) -> dict:
    if "command" in cc_entry:
        args = shlex.split(cc_entry["command"], posix=(os.name != "nt"))
        new = copy.copy(cc_entry)
        del new["command"]
        new["arguments"] = args
        return new
    return cc_entry


def main():
    parser = argparse.ArgumentParser(
            prog='run_on_compile_commands.py',
            description='Runs a clang-based tool on all files in a compile_commands.json database',
    )

    parser.add_argument("-p", help="The folder of compile_commands.json.")
    parser.add_argument("tool", help="The Clang-based tool you want to run. Relative or full path.")
    parser.add_argument("tool_args", nargs=argparse.REMAINDER, help="Arguments passed to the Clang-based tool.")

    values = parser.parse_args()

    compile_commands_path = pathlib.Path(values.p)
    tool_path = pathlib.Path(values.tool)
    tool_args = values.tool_args

    cc_original = json.loads(compile_commands_path.read_text(encoding="utf8"))
    cc_canonicalized = [command_to_args(cc_entry) for cc_entry in cc_original]
    cc_processed = process_compile_commands(cc_canonicalized)

    with tempfile.TemporaryDirectory() as directory:      
        out_file = pathlib.Path(directory) / "compile_commands.json"
        out_file.write_text(json.dumps(cc_processed, indent=2), encoding="utf8")

        cc_files = [cc_entry["file"] for cc_entry in cc_processed]

        subprocess.run([tool_path, "-p", str(out_file.parent), *cc_files, *tool_args])


if __name__ == "__main__":
    main()
