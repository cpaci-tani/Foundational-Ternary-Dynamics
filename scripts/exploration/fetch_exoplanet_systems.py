import urllib.request
import json
import urllib.parse
import os

def fetch_system(hostname):
    query = f"select pl_name,sy_snum,sy_pnum,pl_rade,pl_masse,pl_orbsmax,pl_orbeccen,st_mass,st_rad from ps where hostname='{hostname}'"
    url = 'https://exoplanetarchive.ipac.caltech.edu/TAP/sync?query=' + urllib.parse.quote(query) + '&format=json'
    print('Fetching', hostname, '...')
    try:
        # Include User-Agent to avoid generic 403s
        req = urllib.request.Request(url, headers={'User-Agent': 'Mozilla/5.0'})
        res = urllib.request.urlopen(req)
        data = json.loads(res.read())
        # Deduplicate planet names
        planets = {}
        for row in data:
            pl_name = row['pl_name']
            if pl_name not in planets:
                planets[pl_name] = row
            else:
                p = planets[pl_name]
                # fill missing data from duplicate entries if possible
                for k in ['pl_masse', 'pl_orbsmax', 'pl_orbeccen', 'pl_rade']:
                    if p[k] is None and row[k] is not None:
                        p[k] = row[k]
        
        # Clean up and normalize properties
        # Earth Mass to Solar Mass = 1 / 332946.0487
        # Earth Radius to Solar Radius = 1 / 109.2
        for p in planets.values():
            if p['pl_masse'] is None:
                # Estimate mass using Mass-Radius relation M = R^3 for basic density
                if p['pl_rade'] is not None:
                    p['pl_masse'] = (p['pl_rade'] ** 2.06) if p['pl_rade'] > 2 else (p['pl_rade'] ** 3)
                else:
                    p['pl_masse'] = 1.0 # fallback 1 Earth mass
            
            p['mass_sol'] = p['pl_masse'] / 332946.0487
            if p['st_mass'] is None:
                p['st_mass'] = 1.0
            
            if p['pl_orbsmax'] is None:
                p['pl_orbsmax'] = 1.0 # fallback 1 AU
                
            if p['pl_orbeccen'] is None:
                p['pl_orbeccen'] = 0.0

        return list(planets.values())
    except Exception as e:
        print('Error fetching', hostname, ':', e)
        return []

hosts = ['TRAPPIST-1', 'Kepler-90', 'Kepler-11', 'HR 8799', 'Kepler-20']
output = {}
for h in hosts:
    output[h] = fetch_system(h)

out_file = 'c:/Users/cpaci/Desktop/ftd/engine/web/js/config/exoplanet-seeds.js'
os.makedirs(os.path.dirname(out_file), exist_ok=True)
with open(out_file, 'w') as f:
    f.write('export const EXOPLANET_SEEDS = ' + json.dumps(output, indent=2) + ';\n')
print("Successfully wrote", out_file)
