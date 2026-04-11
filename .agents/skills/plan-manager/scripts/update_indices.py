import os
import re
import hashlib
from datetime import datetime

# Paths
CWD = os.getcwd()
LOG_PATH = os.path.join(CWD, ".agents/project_log.md")
PLANS_ROOT = os.path.join(CWD, ".agents/plans")
DONE_ROOT = os.path.join(PLANS_ROOT, "done")
INDEX_PATH = os.path.join(CWD, ".agents/project_index.md")
ACTIVE_PLANS_PATH = os.path.join(PLANS_ROOT, "ACTIVE_PLANS.md")
ARCHIVED_PLANS_PATH = os.path.join(PLANS_ROOT, "ARCHIVED_PLANS.md")

# Ensure directories exist
os.makedirs(DONE_ROOT, exist_ok=True)

# Keyword Categories for project_index.md
KEYWORDS = {
    "displayManager": ["layout", "widget", "u8g2", "display", "render", "font", "oled"],
    "configManager": ["json", "config", "persistence", "migration", "save", "load", "schema"],
    "webPortal": ["portal", "web", "api", "js", "index.html", "handler", "api/", "webserver"],
    "developmentTools": ["simulator", "wasm", "layoutsim", "script", "mcp", "builder", "gen_layout", "toolchain"],
    "performanceOptimisation": ["oom", "heap", "ram", "freertos", "allocation", "stability", "yield", "refactor", "raii"],
    "hardwareAbstraction": ["wifi", "i2c", "spi", "driver", "chip", "gpio", "hardware"]
}

def get_module_tags(content):
    tags = []
    content_lower = content.lower()
    for cat, kws in KEYWORDS.items():
        if any(kw in content_lower for kw in kws):
            tags.append(cat)
    return tags

def slugify(text):
    return re.sub(r'[\W_]+', '-', text.lower()).strip('-')

def parse_log():
    if not os.path.exists(LOG_PATH):
        return []

    with open(LOG_PATH, 'r') as f:
        content = f.read()

    # Split by "## " at start of line
    sections = re.split(r'\n(?=## )', content)
    
    plans = []
    
    for section in sections:
        if not section.strip() or section.startswith("# Project Log"):
            continue
            
        lines = section.split('\n')
        header_line = lines[0]
        
        # 1. Date
        date_str = "unknown"
        date_match = re.search(r'(\d{4}-\d{2}-\d{2})', header_line)
        if date_match:
            date_str = date_match.group(1)
        else:
            block_date = re.search(r'(?:\*\*Date\*\*|Date):\s*(\d{4}-\d{2}-\d{2})', section)
            if block_date:
                date_str = block_date.group(1)
        
        # 2. Title extraction
        action_match = re.search(r'(?:\*\*Action\*\*|Action):\s*(.*?)(?:\n|$)', section)
        header_title = "Untitled"
        if "-" in header_line:
            header_title = header_line.split("-")[-1].strip()
            header_title = re.sub(r'\(Session .*?\)', '', header_title).strip()

        if action_match:
            title = action_match.group(1).strip()
        elif header_title and not re.match(r'^[a-f0-9\-]{8,}$', header_title.lower()):
            title = header_title
        else:
            title = "Untitled"
        
        # 3. Session IDs
        sessions = []
        session_header_match = re.search(r'\(Session (.*?)\)', header_line)
        if session_header_match:
            sessions.append(session_header_match.group(1))
        session_block_matches = re.findall(r'(?:\*\*Session\*\*|Session):\s*([a-f0-9\-]{8,})', section)
        sessions.extend(session_block_matches)
        
        # Plan ID
        plan_id_match = re.search(r'\[\.agents/plans/(?:done/)?([a-f0-9\-]{8,})/?\]', section)
        header_id_match = re.search(r'Plan\s+([a-f0-9\-]{8,})', header_line)
        plan_id = None
        if plan_id_match: plan_id = plan_id_match.group(1)
        elif header_id_match: plan_id = header_id_match.group(1)
        elif sessions: plan_id = sessions[0]
            
        if not plan_id:
            plan_id = f"legacy-{slugify(title)[:30]}-{hashlib.md5(title.encode()).hexdigest()[:6]}"

        # 4. Commits
        commits = []
        header_commit = re.search(r'^##\s+\[([a-f0-9]{7,})\]', header_line)
        if header_commit: commits.append(header_commit.group(1))
        commit_list_matches = re.findall(r'(?:\*\*Commit\*\*|Commit|Generated commits?|commits?):\s*([a-f0-9,\s&()\-]+)', section, re.IGNORECASE)
        for cand in commit_list_matches:
            commits.extend(re.findall(r'\b[a-f0-9]{7,}\b', cand))
        inline_commits = re.findall(r'Commit\s+([a-f0-9]{7,})', section, re.IGNORECASE)
        commits.extend(inline_commits)
        commits = sorted(list(set(commits)))
        sessions = sorted(list(set(sessions)))
        
        # 5. Summary
        summary = ""
        summary_items = [
            r'-\s+\*\*Summary\*\*:\s+(.*?)(?:\n-|$)',
            r'### Session Summary\n(.*?)(?:\n###|$)',
            r'\*\*Summary\*\*:\s+(.*?)(?:\n\*\*|$)'
        ]
        for pattern in summary_items:
            sm = re.search(pattern, section, re.DOTALL)
            if sm:
                summary = sm.group(1).strip()
                break
        if not summary:
            summary = "\n".join(lines[1:4]).strip()
        
        # Heuristic title recovery
        if title == "Untitled" or re.match(r'^[a-f0-9\-]{8,}$', title.lower()):
            clean_summary = re.sub(r'#.*?\n', '', summary).strip()
            if clean_summary:
                title = " ".join(clean_summary.split()[:10]) + "..."
            else:
                title = "Unknown Session"
        
        # Decisions
        decisions = ""
        decisions_match = re.search(r'### Key Decisions\n(.*?)(?:\n###|$)', section, re.DOTALL)
        if decisions_match: decisions = decisions_match.group(1).strip()
        
        is_done = "done" in section.lower() or "PLAN.md" not in section
        dest_dir = os.path.join(DONE_ROOT, plan_id) if is_done else os.path.join(PLANS_ROOT, plan_id)
        
        plans.append({
            "id": plan_id,
            "title": title,
            "date": date_str,
            "summary": summary,
            "commits": commits,
            "sessions": sessions,
            "decisions": decisions,
            "path": dest_dir,
            "status": "DONE" if is_done else "SAVED",
            "tags": get_module_tags(section),
            "source": "log"
        })
        
    return plans

def migrate_plans(plans):
    for plan in plans:
        os.makedirs(plan["path"], exist_ok=True)
        plan_file = os.path.join(plan["path"], "PLAN.md")
        sessions_file = os.path.join(plan["path"], "sessions.md")
        
        # 1. Update/Create sessions.md
        existing_sessions = []
        if os.path.exists(sessions_file):
            with open(sessions_file, 'r') as f:
                existing_sessions = [s.strip() for s in f.readlines() if s.strip()]
        
        merged_sessions = sorted(list(set(existing_sessions + plan["sessions"])))
        if merged_sessions:
            with open(sessions_file, 'w') as f:
                f.write("\n".join(merged_sessions) + "\n")

        # 2. Write PLAN.md (NO sessions in YAML)
        desc = plan["summary"][:200].replace('"', '\\"').replace('\n', ' ') + ("..." if len(plan["summary"]) > 200 else "")
        frontmatter = f"""---
name: "{plan['title']}"
description: "{desc}"
created: "{plan['date']}"
status: "{plan['status']}"
commits: {plan['commits']}
---"""
        body = f"""{frontmatter}\n\n# Summary\n{plan["summary"]}\n\n"""
        if plan["decisions"]: body += f"## Key Decisions\n{plan['decisions']}\n\n"
        body += "## Technical Context\n"
        for cf in ["sessions.md", "context_bridge.md", "task.md"]:
            if os.path.exists(os.path.join(plan["path"], cf)):
                body += f"- [{cf}]({cf})\n"
        with open(plan_file, 'w') as f:
            f.write(body)

def generate_indices(plans):
    plans.sort(key=lambda x: x['date'], reverse=True)
    
    # ACTIVE_PLANS.md (Dashboard View)
    active = [p for p in plans if p["status"] != "DONE"]
    
    def get_status_group(status):
        if status == "WIP": return "## 🔨 Work In Progress"
        if status == "READY": return "## ⏳ Pending Queue"
        return "## 💾 Saved (Unqueued) Plans"

    status_groups = {"WIP": [], "READY": [], "SAVED": []}
    for p in active:
        s = p["status"] if p["status"] in status_groups else "SAVED"
        status_groups[s].append(p)

    with open(ACTIVE_PLANS_PATH, 'w') as f:
        f.write("# Project Dashboard: Active Plans\n\n")
        f.write("Current status of active sessions, queued tasks, and saved drafts.\n\n")
        
        for status in ["WIP", "READY", "SAVED"]:
            f.write(f"{get_status_group(status)}\n")
            if status == "SAVED": f.write("*Tip: Use `/plan-queue [Plan ID]` to mark a plan as READY.*\n\n")
            
            p_list = status_groups[status]
            if not p_list:
                f.write("_No plans found in this state._\n\n")
            else:
                f.write("| Date | Title | Summary | Plan Link |\n| :--- | :--- | :--- | :--- |\n")
                for p in p_list:
                    short_summary = p['summary'].split('.')[0].strip()[:100] + "..."
                    f.write(f"| {p['date']} | {p['title']} | {short_summary} | [{p['id']}/PLAN.md]({p['id']}/PLAN.md) |\n")
                f.write("\n")
        
        f.write("\n*Tip: Use **`/plan-load [Plan ID]`** to resume a session.*")

    # ARCHIVED_PLANS.md
    archived = [p for p in plans if p["status"] == "DONE"]
    def get_group_key(date_str):
        if not date_str or date_str == "unknown": return "Legacy / Unknown"
        try:
            return datetime.strptime(date_str, "%Y-%m-%d").strftime("%Y-%b")
        except: return "Legacy / Unknown"

    groups = {}
    for p in archived:
        gk = get_group_key(p['date'])
        if gk not in groups: groups[gk] = []
        groups[gk].append(p)

    sorted_group_keys = sorted(groups.keys(), reverse=True)
    if "Legacy / Unknown" in sorted_group_keys:
        sorted_group_keys.remove("Legacy / Unknown")
        sorted_group_keys.append("Legacy / Unknown")

    with open(ARCHIVED_PLANS_PATH, 'w') as f:
        f.write("# Historical Ledger\n\nComprehensive history of completed plans, grouped by completion date.\n\n")
        for gk in sorted_group_keys:
            f.write(f"## {gk}\n\n| Date | Title | Key Impact | Plan Link |\n| :--- | :--- | :--- | :--- |\n")
            for p in groups[gk]:
                impact = p['summary'].split('.')[0].strip() + '.'
                if len(impact) > 150: impact = impact[:147] + "..."
                f.write(f"| {p['date']} | {p['title']} | {impact} | [done/{p['id']}/](done/{p['id']}/PLAN.md) |\n")
            f.write("\n")

    # project_index.md
    with open(INDEX_PATH, 'w') as f:
        f.write("# Project Index: Departures Board\n\n## State of the Union\nThis project focuses on providing real-time transit information on embedded hardware (ESP32) with a focus on UI/UX aesthetics and memory stability.\n\n## Module & Tool Index\n\n")
        categories = [
            ("displayManager", "🖥️ displayManager"), ("configManager", "⚙️ configManager"),
            ("webPortal", "🌐 webPortal"), ("developmentTools", "🛠️ developmentTools"),
            ("performanceOptimisation", "⚡ performanceOptimisation"), ("hardwareAbstraction", "🔌 hardwareAbstraction"),
            ("general", "📁 General / Core Architecture")
        ]
        for module, label in categories:
            f.write(f"### {label}\n")
            if module == "general":
                mod_active = [p for p in active if not p["tags"]]
                mod_archived = [p for p in archived if not p["tags"]]
            else:
                mod_active = [p for p in active if module in p["tags"]]
                mod_archived = [p for p in archived if module in p["tags"]]
            
            if mod_active:
                f.write("- **Active Plans**:\n")
                for p in mod_active: f.write(f"  - [.agents/plans/{p['id']}/]({p['id']}/PLAN.md) ({p['title']})\n")
            if mod_archived:
                f.write("- **Archive**:\n")
                for p in mod_archived:
                    link_prefix = "done/" if p['status'] == "DONE" else ""
                    f.write(f"  - [.agents/plans/{link_prefix}{p['id']}/]({link_prefix}{p['id']}/PLAN.md) ({p['title']})\n")
            if not mod_active and not mod_archived: f.write("- *No history recorded for this module.*\n")
            f.write("\n")
        f.write("## Global Activity Ledgers\n- [Active Working Set](plans/ACTIVE_PLANS.md)\n- [Historical Ledger](plans/ARCHIVED_PLANS.md)\n")

if __name__ == "__main__":
    all_plans_dict = {}
    log_plans = parse_log()
    for p in log_plans: all_plans_dict[p["id"]] = p

    for root_dir in [PLANS_ROOT, DONE_ROOT]:
        if not os.path.exists(root_dir): continue
        for folder in os.listdir(root_dir):
            folder_path = os.path.join(root_dir, folder)
            plan_file = os.path.join(folder_path, "PLAN.md")
            if os.path.isdir(folder_path) and os.path.exists(plan_file) and folder not in all_plans_dict:
                with open(plan_file, 'r') as f: content = f.read()
                p_name, p_date, p_status, p_summary, p_sessions, p_commits = folder, "unknown", "DONE" if "done" in root_dir else "SAVED", "", [], []
                fm_match = re.search(r'---\n(.*?)\n---', content, re.DOTALL)
                if fm_match:
                    fm = fm_match.group(1)
                    nm = re.search(r'name:\s*"(.*?)"', fm) or re.search(r'name:\s*(.*?)(?:\n|$)', fm)
                    if nm: p_name = nm.group(1).strip().strip('"')
                    dm = re.search(r'created:\s*"(.*?)"', fm) or re.search(r'created:\s*(.*?)(?:\n|$)', fm)
                    if dm: p_date = dm.group(1).strip().split('T')[0]
                    sm = re.search(r'status:\s*"(.*?)"', fm) or re.search(r'status:\s*(.*?)(?:\n|$)', fm)
                    if sm: p_status = sm.group(1).strip().strip('"')
                    sesm = re.search(r'sessions:\s*\[(.*?)\]', fm)
                    if sesm: p_sessions = [s.strip().strip("'").strip('"') for s in sesm.group(1).split(',') if s.strip()]
                    comm = re.search(r'commits:\s*\[(.*?)\]', fm)
                    if comm: p_commits = [c.strip().strip("'").strip('"') for c in comm.group(1).split(',') if c.strip()]
                
                sum_m = re.search(r'# Summary\n(.*?)(?:\n##|$)', content, re.DOTALL)
                p_summary = sum_m.group(1).strip() if sum_m else content.split('---')[-1].strip().split('\n')[0]
                
                all_plans_dict[folder] = {
                    "id": folder, "title": p_name, "date": p_date, "summary": p_summary, "commits": p_commits,
                    "sessions": p_sessions, "decisions": "", "path": folder_path, "status": p_status,
                    "tags": get_module_tags(content), "source": "disk"
                }

    all_plans_list = list(all_plans_dict.values())
    migrate_plans(all_plans_list)
    generate_indices(all_plans_list)
    print(f"Final Index contains {len(all_plans_list)} unique plans.")
    orphans = [p["id"] for p in all_plans_list if p.get("source") != "log"]
    if orphans:
        print(f"\nAUDIT REPORT: Found {len(orphans)} plans on disk NOT present in project_log.md:")
        for oid in orphans: print(f" - {oid}")
