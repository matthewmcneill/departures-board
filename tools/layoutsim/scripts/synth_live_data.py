#!/usr/bin/env python3
"""
Departures Board (c) 2025-2026 Gadec Software
Refactored for v3.0 by Matt McNeill 2026 CB Labs

Module: tools/layoutsim/scripts/synth_live_data.py
Description: Python synthesizer that interacts with live National Rail (Darwin) and TfL APIs to populate realistic local layout tester feeds in `mock_data/`.
"""

import os
import sys
import json
import urllib.request
import xml.etree.ElementTree as ET
import ssl

# Bypass macOS python local issuer limitations for local data fetching
ssl_ctx = ssl.create_default_context()
ssl_ctx.check_hostname = False
ssl_ctx.verify_mode = ssl.CERT_NONE

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.dirname(SCRIPT_DIR)))
ENV_FILE = os.path.join(PROJECT_ROOT, ".env")
MOCK_DIR = os.path.join(PROJECT_ROOT, "tools", "layoutsim", "mock_data")

def load_env():
    env = {}
    if os.path.exists(ENV_FILE):
        with open(ENV_FILE, "r") as f:
            for line in f:
                if "=" in line and not line.strip().startswith("#"):
                    k, v = line.strip().split("=", 1)
                    env[k] = v
    return env

def save_env(env):
    with open(ENV_FILE, "w") as f:
        for k, v in env.items():
            f.write(f"{k}={v}\n")

def get_api_keys():
    env = load_env()
    save_needed = False
    
    nr_key = env.get("DARWIN_TOKEN")
    if not nr_key:
        print("Missing National Rail Darwin API Token.")
        print("You can register for one at: http://realtime.nationalrail.co.uk/OpenLDBWSRegistration/")
        nr_key = input("Enter your DARWIN_TOKEN: ").strip()
        env["DARWIN_TOKEN"] = nr_key
        save_needed = True
        
    tfl_key = env.get("TFL_APP_KEY")
    if not tfl_key:
        print("\nMissing Transport for London API Key.")
        print("You can register for one at: https://api.portal.tfl.gov.uk/")
        tfl_key = input("Enter your TFL_APP_KEY (or press Enter to run unauthenticated with tight rate limits): ").strip()
        if tfl_key:
            env["TFL_APP_KEY"] = tfl_key
            save_needed = True

    if save_needed:
        save_env(env)
        print(f"Saved your keys locally to {ENV_FILE}. You won't be prompted again.\n")
        
    return env.get("DARWIN_TOKEN", ""), env.get("TFL_APP_KEY", "")

def fetch_rss(url):
    print(f"Fetching RSS feed from {url}...")
    headers = {'User-Agent': 'ESP32_Test_Rig'}
    req = urllib.request.Request(url, headers=headers)
    try:
        with urllib.request.urlopen(req, context=ssl_ctx) as resp:
            xml_str = resp.read().decode('utf-8')
    except Exception as e:
        print(f"Failed to fetch RSS: {e}")
        return []

    try:
        root = ET.fromstring(xml_str)
        items = root.findall('.//item')
        titles = []
        for i, item in enumerate(items):
            if i >= 5: break # Cap to simulate ESP32 memory limit
            title = item.find('title')
            if title is not None and title.text:
                titles.append(title.text)
        return titles
    except Exception as e:
        print(f"Failed to parse RSS XML: {e}")
        return []

def fetch_tfl(naptan_id, app_key):
    print(f"Fetching live TfL data for {naptan_id}...")
    headers = {'User-Agent': 'ESP32_Test_Rig'}
    
    # Arrivals
    url_arr = f"https://api.tfl.gov.uk/StopPoint/{naptan_id}/Arrivals"
    if app_key: url_arr += f"?app_key={app_key}"
    
    req = urllib.request.Request(url_arr, headers=headers)
    try:
        with urllib.request.urlopen(req, context=ssl_ctx) as resp:
            arrivals = json.loads(resp.read().decode('utf-8'))
    except Exception as e:
        print(f"Failed to fetch TfL arrivals: {e}")
        return None
        
    # Disruptions
    msgs = []
    url_dis = f"https://api.tfl.gov.uk/StopPoint/{naptan_id}/Disruption?getFamily=true&flattenResponse=true"
    if app_key: url_dis += f"&app_key={app_key}"
    
    try:
        req = urllib.request.Request(url_dis, headers=headers)
        with urllib.request.urlopen(req, context=ssl_ctx) as resp:
            dis_data = json.loads(resp.read().decode('utf-8'))
            for d in dis_data:
                if 'description' in d:
                    # simplistic strip of newlines like C++ port does
                    msgs.append(d['description'].replace('\\n', ' '))
    except Exception as e:
        print(f"Warning: Failed to fetch TfL disruptions: {e}")

    # Build schema
    # Sort by timeToStation
    arrivals.sort(key=lambda x: x.get('timeToStation', 9999))
    
    title = "London Underground"
    if arrivals:
        # e.g. "Victoria Underground Station"
        raw_name = arrivals[0].get('stationName', 'Unknown Station')
        if " Underground" in raw_name: raw_name = raw_name.replace(" Underground Station", "")
        title = raw_name

    services = []
    for i, a in enumerate(arrivals[:10]):
        ord_str = str(i+1)
        mins = a.get('timeToStation', 0) // 60
        time_str = "Due" if mins == 0 else f"{mins} mins"
        dest = a.get('destinationName', 'Unknown')
        if " Underground Station" in dest: dest = dest.replace(" Underground Station", "")
        plat = "" # TfL doesn't always strictly show platform number clearly in simple UI without parsing platformName
        if 'platformName' in a:
            p = a['platformName']
            if "Platform" in p: plat = p.split("Platform")[-1].strip()
        
        status = a.get('lineName', '')
        services.append([ord_str, time_str, dest, plat, status])

    payload = {
        "header": {
            "title": title,
            "callingPoint": "Transport for London",
            "platform": ""
        },
        "services": services,
        "weather": {"id": 800, "isNight": False},
        "messages": msgs[:10]
    }
    return payload

def fetch_national_rail(crs, token):
    print(f"Fetching live National Rail (Darwin) data for {crs}...")
    
    xml_req = f"""<soap-env:Envelope xmlns:soap-env="http://schemas.xmlsoap.org/soap/envelope/" xmlns:ns1="http://thalesgroup.com/RTTI/2013-11-28/Token/types" xmlns:ns2="http://thalesgroup.com/RTTI/2021-11-01/ldb/">
<soap-env:Header><ns1:AccessToken><ns1:TokenValue>{token}</ns1:TokenValue></ns1:AccessToken></soap-env:Header>
<soap-env:Body><ns2:GetDepBoardWithDetailsRequest><ns2:numRows>10</ns2:numRows><ns2:crs>{crs}</ns2:crs>
</ns2:GetDepBoardWithDetailsRequest></soap-env:Body></soap-env:Envelope>"""

    url = "https://lite.realtime.nationalrail.co.uk/OpenLDBWS/ldb12.asmx"
    headers = {
        'Content-Type': 'text/xml; charset=utf-8',
        'SOAPAction': '"http://thalesgroup.com/RTTI/2015-05-14/ldb/GetDepBoardWithDetails"',
        'User-Agent': 'ESP32_Test_Rig'
    }
    
    req = urllib.request.Request(url, data=xml_req.encode('utf-8'), headers=headers)
    try:
        with urllib.request.urlopen(req, context=ssl_ctx) as resp:
            resp_bytes = resp.read()
    except Exception as e:
        print(f"Failed to fetch National Rail SOAP: {e}")
        return None

    # Strip XML Namespaces to make ElementTree queries sane
    xml_str = resp_bytes.decode('utf-8')
    import re
    xml_str = re.sub(r'\sxmlns="[^"]+"', '', xml_str, count=1)
    
    # We just manually extract without strict schema because it's a test synth
    root = ET.fromstring(xml_str)
    
    # Simple recursive namespace stripper logic for ElementTree
    for elem in root.iter():
        if '}' in elem.tag:
            elem.tag = elem.tag.split('}', 1)[1]

    title = "Unknown"
    loc_node = root.find('.//locationName')
    if loc_node is not None and loc_node.text:
        title = loc_node.text

    services = []
    svcs_node = root.findall('.//service')
    for i, s in enumerate(svcs_node):
        ord_str = f"{i+1}{'st' if i==0 else 'nd' if i==1 else 'rd' if i==2 else 'th'}"
        std = s.find('.//std')
        etd = s.find('.//etd')
        time_str = std.text if std is not None else ""
        status = etd.text if etd is not None else "On time"
        
        plat_node = s.find('.//platform')
        plat = plat_node.text if plat_node is not None else ""
        
        dest = ""
        dest_node = s.find('.//destination//locationName')
        if dest_node is not None: dest = dest_node.text
            
        services.append([ord_str, time_str, dest, plat, status])
        
    msgs = []
    # Disruptions
    msg_nodes = root.findall('.//message')
    for m in msg_nodes:
        # Extract text omitting inner HTML
        txt = ''.join(m.itertext()).strip()
        txt = txt.replace('\\n', ' ')
        msgs.append(txt)

    payload = {
        "header": {
            "title": title,
            "callingPoint": "National Rail",
            "platform": ""
        },
        "services": services,
        "weather": {"id": 200, "isNight": False}, # Thunderstorm to mix it up
        "messages": msgs[:10]
    }
    return payload

def main():
    print("--- Live Data Synthesizer ---")
    nr_t, tfl_k = get_api_keys()
    
    os.makedirs(MOCK_DIR, exist_ok=True)
    
    # Grab BBC Headlines
    bbc_news = fetch_rss("http://feeds.bbci.co.uk/news/rss.xml")
    
    # Fetch TfL (Victoria Station)
    tfl_payload = fetch_tfl("940GZZLUVIC", tfl_k)
    if tfl_payload:
        tfl_payload["messages"].extend(bbc_news)
        p = os.path.join(MOCK_DIR, "tflBoard.json")
        with open(p, "w") as f: json.dump(tfl_payload, f, indent=4)
        print(f"SUCCESS: Wrote TfL mock data to {p}")
        
    # Fetch Darwin (Euston)
    if nr_t:
        nr_payload = fetch_national_rail("EUS", nr_t)
        if nr_payload:
            nr_payload["messages"].extend(bbc_news)
            p = os.path.join(MOCK_DIR, "nationalRailBoard.json")
            with open(p, "w") as f: json.dump(nr_payload, f, indent=4)
            print(f"SUCCESS: Wrote National Rail mock data to {p}")
    else:
        print("Skipping National Rail; DARWIN_TOKEN is missing.")

if __name__ == "__main__":
    main()
