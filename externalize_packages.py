#!/usr/bin/env python3
"""
External packages manager entrypoint.
Execute the vendoring + generation pipeline.
"""

import runpy
import sys

if __name__ == "__main__":
    sys.exit(runpy.run_path("build_shell_from_json.py", run_name="__main__") or 0)

