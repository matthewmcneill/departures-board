import os
import glob
import re
import sys

def list_plans():
    show_all = "--all" in sys.argv
    use_stdout = "--stdout" in sys.argv

    script_dir = os.path.dirname(os.path.abspath(__file__))
    base_dir = os.path.abspath(os.path.join(script_dir, "../../../plans"))
    lockfile = os.path.join(base_dir, "lock.md")
    output_file = os.path.join(base_dir, "plan_list_view.md")
    
    output_lines = []
    
    # Print the Lock Status first
    output_lines.append("### 🔒 Global Hardware Lock")
    if os.path.exists(lockfile):
        with open(lockfile, "r") as f:
            lines = f.readlines()
            in_lock = False
            for line in lines:
                if line.startswith("## Lock Status"):
                    in_lock = True
                    continue
                if in_lock:
                    if line.startswith("##"):
                        break
                    if line.strip():
                        output_lines.append(line.strip())
            
            has_lock = any("NONE" not in l and "Locked By" in l for l in lines)
            if not has_lock:
                output_lines.append("\n> **Note**: The hardware and build systems are currently **FREE** and available for `pio run`.")
            else:
                output_lines.append("\n> **WARNING**: The hardware is currently in use. Please respect the mutex.")
    else:
        output_lines.append("- **Locked By**: NONE (lockfile missing)")
    output_lines.append("\n---")
    
    # We find all PLAN.md files.
    if show_all:
        plan_files = glob.glob(f"{base_dir}/**/PLAN.md", recursive=True)
    else:
        plan_files = glob.glob(f"{base_dir}/*/PLAN.md")
    
    pending_lines = []
    wip_lines = []
    saved_lines = []
    
    pending_lines.append("| Plan ID | Title | Description | Created |")
    pending_lines.append("|:---|:---|:---|:---|")
    
    wip_lines.append("| Plan ID | Title | Description | Created |")
    wip_lines.append("|:---|:---|:---|:---|")
    
    saved_lines.append("| Plan ID | Title | Description | Created |")
    saved_lines.append("|:---|:---|:---|:---|")
    
    wip_count = 0
    pending_count = 0
    saved_count = 0
    
    for pf in sorted(plan_files):
        if not show_all and "/done/" in pf:
            continue
            
        plan_dir = os.path.dirname(pf)
        plan_id = os.path.basename(plan_dir)
            
        with open(pf, "r") as f:
            content = f.read()
            match = re.search(r'^---\n(.*?)\n---', content, re.DOTALL)
            if match:
                yml = match.group(1)
                
                name = ""
                desc = ""
                created = ""
                status = ""
                
                for line in yml.split('\n'):
                    if line.startswith("name:"):
                        name = line[5:].strip()
                    elif line.startswith("description:"):
                        desc = line[12:].strip()
                    elif line.startswith("created:"):
                        created = line[8:].strip()
                    elif line.startswith("status:"):
                        status = line[7:].strip()
                
                if len(desc) > 50:
                    desc = desc[:47] + "..."
                
                row = f"| `{plan_id}` | {name} | {desc} | {created} |"
                
                if status == "WIP":
                    wip_lines.append(row)
                    wip_count += 1
                elif status == "READY":
                    pending_lines.append(row)
                    pending_count += 1
                elif status == "SAVED":
                    saved_lines.append(row)
                    saved_count += 1
                elif status == "DONE" and show_all:
                    pending_lines.append(f"| `{plan_id}` | **DONE**: {name} | {desc} | {created} |")
                    pending_count += 1
                
    if saved_count > 0:
        output_lines.append("\n### 💾 Saved (Unqueued) Plans")
        output_lines.append("*Tip: Use `/plan-queue [Plan ID]` to mark a plan as READY.*")
        output_lines.extend(saved_lines)

    if pending_count > 0:
        output_lines.append("\n### ⏳ Pending Queue")
        output_lines.extend(pending_lines)
    else:
        output_lines.append("\n### ⏳ Pending Queue")
        output_lines.append("*Queue is empty.*")
        
    if wip_count > 0:
        output_lines.append("\n### 🔨 Work In Progress Sessions")
        output_lines.extend(wip_lines)
    else:
        output_lines.append("\n### 🔨 Work In Progress Sessions")
        output_lines.append("*No active sessions.*")
        
    output_lines.append("\n*Tip: Use **`/plan-load [Plan ID]`** to resume a session's rich context. Add **`--all`** to see archives.*")
    
    if use_stdout:
        print("\n".join(output_lines))
    else:
        with open(output_file, "w") as f:
            f.write("\n".join(output_lines))
        print(f"Generated successfully to {output_file}")

if __name__ == "__main__":

    list_plans()
