### DNS Resolver: Iterative and Recursive Lookups
### 1. Introduction
This Python code demonstrates two methods of DNS resolution to get IP addresses:

### 1.1 Iterative DNS Lookup and its Flowchart 

Manually queries DNS servers, starting from root servers and moving down through TLD and authoritative servers.
Provides visibility into each step of the resolution process.
 ┌─────────────────────────┐
 │       Start /           │
 │  Receive Domain Input   │
 └─────────────────────────┘
             │
             ▼
 ┌─────────────────────────┐
 │ Load Root Server IPs    │
 │ (e.g., a.root-servers..)│
 └─────────────────────────┘ 
             │
             ▼
 ┌─────────────────────────┐
 │ Query Root Server for   │
 │ the Domain (A record)   │
 └─────────────────────────┘
             │
   ┌─────────┴───────────────────────────────────────┐
   │                                                 │
   │ If ANSWER SECTION found? (A record returned?)   │
   │    ┌──────────────────────┐                     │
   │  Yes                      │ No                  │
   │    ▼                      │                     │
   │  ┌──────────────────────┐ │                     │
   │  │ Return IP to User    │ │                     │
   │  │ (Success)            │ │                     │
   │  └──────────────────────┘ │                     │
   │                           │                     │
   └───────────────────────────┘                     │
             │                                       │
             ▼                                       │
   ┌─────────────────────────┐                       │
   │ Extract NS Records from │                       │
   │ Authority Section       │                       |
   └─────────────────────────┘                       |
             │                                       |
             ▼                                       |
 ┌─────────────────────────┐                         |
 │ Resolve NS Hostnames to │                         |
 │ IPs (if needed)         │                         |
 └─────────────────────────┘                         |
             │                                       |
             ▼                                       |
 ┌─────────────────────────┐                         |
 │ Query Next Nameserver   │                         |
 │ (TLD or Authoritative)  │                         |
 └─────────────────────────┘                         |
             │                                       |
             ▼                                       |
       (Repeat Steps)                                |

### 1.2 Recursive DNS Lookup and its Flowchart

Uses the system’s default DNS resolver (e.g., a local ISP resolver or a public resolver like Google DNS).
The DNS library (dnspython) handles the recursion internally, so the user sees only the final answer or an error.
 ┌─────────────────────────┐
 │       Start /           │
 │  Receive Domain Input   │
 └─────────────────────────┘
             │
             ▼
 ┌─────────────────────────┐
 │ Use System’s DNS        │
 │ Resolver (Recursive)    │
 │  e.g., dns.resolver API │
 └─────────────────────────┘
             │
   ┌─────────┴─────────────────────────┐
   │                                   │
   │  If resolver returns A record?    │
   │    ┌─────────────────────────┐    │
   │  Yes                         │ No │
   │    ▼                         │    │
   │  ┌─────────────────────────┐ │    │
   │  │ Return IP to User       │ │    │
   │  │ (Success)               │ │    │
   │  └─────────────────────────┘ │    │
   │                              │    │
   └──────────────────────────────┘    │
             │                         │
             ▼                         │
       (End / Failure)                 │

### 2. Features
Root Servers: A small subset of known root server IPs is used as the starting point for iterative lookups.
Timeouts: Configurable TIMEOUT for DNS queries.
Error Handling: Catches exceptions like unreachable servers, invalid domain names, and timeouts.
Debug Messages: Prints debug statements indicating the servers queried, nameservers extracted, and final IPs found.
### 3. File Contents
dns_resolver.py
Global Variables

ROOT_SERVERS: Dictionary containing some root server IPs.
TIMEOUT: Query timeout in seconds (default is 3 seconds).
Functions

send_dns_query(server, domain)

Constructs a DNS query (type A) for the specified domain and sends it to server.
Uses dns.query.udp() with a specified timeout.
Returns a DNS response object or None if an error occurs.
extract_next_nameservers(response)

Looks at the authority section of the DNS response to find NS (nameserver) records.
For each NS record, it attempts to resolve the hostname to an IP address.
Returns a list of resolved IP addresses.
iterative_dns_lookup(domain)

Implements the iterative resolution process.
Begins with the known root server IPs and queries them for the domain’s A record.
If no direct A record is returned, it extracts NS records from the authority section and queries those servers next (TLD or authoritative servers).
Continues until it finds an A record or runs out of nameservers.
recursive_dns_lookup(domain)

Relies on the system’s DNS resolver (using dns.resolver.resolve()) to perform the resolution.
Prints the final IP addresses or an error.
Main Execution

Reads command-line arguments to determine the mode (iterative or recursive) and the domain to be resolved.
Measures the execution time.
Calls either iterative_dns_lookup(domain) or recursive_dns_lookup(domain).
### 4. How to Run
4.1. Prerequisites(install python 3)
Python 3.x: Ensure you have Python 3 installed (e.g., Python 3.8+).
dnspython Library: Install using
pip install dnspython
4.2. Running the Script
Clone or Download the code (dns_resolver.py) into your working directory.

Open the working directory in Terminal .

Run the Script with one of the following commands:

### Iterative Mode:(if you want to go iterative)
python3 dnsresolver.py iterative <domain Name(eg hello.iitk.ac.in)>
Example:
python3 dns_resolver.py iterative xyzw.com
This will manually query root, TLD, and authoritative servers step by step.

### Recursive Mode:(if you want  go recursive )

python3 dns_resolver.py recursive <domain Name(eg hello.iitk.ac.in)>
Example:

python3 dns_resolver.py recursive xyzw.com
This uses the system’s resolver and is typically much faster and simpler to run.

### 4.3. Sample Outputs(For hello.iitk.ac.in)
 ### Iterative Example:

[Iterative DNS Lookup] Resolving hello.iitk.ac.in
[DEBUG] Querying ROOT server (198.41.0.4) - SUCCESS
Extracted NS hostname: ns2.registry.in.
Extracted NS hostname: ns5.registry.in.
Extracted NS hostname: ns4.registry.in.
Extracted NS hostname: ns1.registry.in.
Extracted NS hostname: ns6.registry.in.
Extracted NS hostname: ns3.registry.in.
Resolved ns3.registry.in. to 37.209.196.12
Resolved ns3.registry.in. to 37.209.196.12
Resolved ns3.registry.in. to 37.209.196.12
Resolved ns3.registry.in. to 37.209.196.12
Resolved ns3.registry.in. to 37.209.196.12
Resolved ns3.registry.in. to 37.209.196.12
[DEBUG] Querying TLD server (37.209.196.12) - SUCCESS
Extracted NS hostname: ns1.iitk.ac.in.
Extracted NS hostname: ns2.iitk.ac.in.
Extracted NS hostname: proxy.iitk.ac.in.
Resolved proxy.iitk.ac.in. to 172.31.1.210
Resolved proxy.iitk.ac.in. to 172.31.1.210
Resolved proxy.iitk.ac.in. to 172.31.1.210
[DEBUG] Querying AUTH server (172.31.1.210) - SUCCESS
[SUCCESS] hello.iitk.ac.in -> 202.3.77.62
Time taken: 0.213 seconds
### Recursive Example:

[Recursive DNS Lookup] Resolving hello.iitk.ac.in
[SUCCESS] hello.iitk.ac.in -> 172.31.101.67
Time taken: 0.014 seconds
### Flowchart for How Program Runs foe our given example of hello.iitk.ac.in
                           ┌─────────────┐
                           │   Start     │
                           └──────┬──────┘
                                  │
                                  ▼
                   ┌─────────────────────────────┐
                   │ Parse Command-Line Arguments│
                   │  (mode and domain input)    │
                   │ e.g., "iterative" or        │
                   │       "recursive"           │
                   │ and "hello.iitk.ac.in"      │
                   └────────────┬────────────────┘
                                │
                                ▼
                   ┌─────────────────────────────┐
                   │    Check Selected Mode      │
                   └────────────┬────────────────┘
                        ┌─────┴─────┐
                        │           │
                      Yes         No (recursive)
                        │           │
                        ▼           ▼
         ┌─────────────────────────┐   ┌─────────────────────────────┐
         │ [Iterative DNS Lookup]  │   │ [Recursive DNS Lookup]      │
         │ Resolving hello.iitk.ac.in│   │ Resolving hello.iitk.ac.in│
         └────────────┬────────────┘   └────────────┬────────────────┘
                      │                             │
                      ▼                             ▼
         ┌─────────────────────────┐   ┌─────────────────────────────┐
         │ Query ROOT server       │   │ Use system’s DNS resolver   │
         │ (198.41.0.4)            │   │ (dns.resolver.resolve)      │
         └────────────┬────────────┘   └────────────┬────────────────┘
                      │                             │
                      ▼                             ▼
         ┌─────────────────────────┐   ┌─────────────────────────────┐
         │ [DEBUG] Query Success   │   │ [SUCCESS] Domain resolved:  │
         │ for ROOT server         │   │ hello.iitk.ac.in ->         │
         │                         │   │ 172.31.101.67               │
         └────────────┬────────────┘   └────────────┬────────────────┘
                      │                             │
                      ▼                             ▼
         ┌─────────────────────────┐   ┌─────────────────────────────┐
         │ Extract NS records from │   │ Print Execution Time        │
         │ ROOT response:          │   │ (e.g., 0.014 sec)           │
         │ ns2.registry.in,        │   └────────────┬────────────────┘
         │ ns5.registry.in,        │                │
         │ ns4.registry.in,        │                ▼
         │ ns1.registry.in,        │           ┌─────────────┐
         │ ns6.registry.in, ns3.registry.in│   │    End      │
         └────────────┬────────────┘           └─────────────┘
                      │
                      ▼
         ┌─────────────────────────┐
         │ Resolve NS hostnames    │
         │ to IP(s):               │
         │ ns3.registry.in ->      │
         │ 37.209.196.12           │
         └────────────┬────────────┘
                      │
                      ▼
         ┌─────────────────────────┐
         │ [DEBUG] Query TLD       │
         │ server (37.209.196.12)  │
         │ - SUCCESS               │
         └────────────┬────────────┘
                      │
                      ▼
         ┌─────────────────────────┐
         │ Extract NS records from │
         │ TLD response:           │
         │ ns1.iitk.ac.in,         │
         │ ns2.iitk.ac.in,         │
         │ proxy.iitk.ac.in        │
         └────────────┬────────────┘
                      │
                      ▼
         ┌─────────────────────────┐
         │ Resolve NS hostnames    │
         │ to IP(s):               │
         │ proxy.iitk.ac.in ->     │
         │ 172.31.1.210            │
         └────────────┬────────────┘
                      │
                      ▼
         ┌─────────────────────────┐
         │ [DEBUG] Query AUTH      │
         │ server (172.31.1.210)   │
         │ - SUCCESS               │
         └────────────┬────────────┘
                      │
                      ▼
         ┌─────────────────────────┐
         │ [SUCCESS] Domain        │
         │ resolved:               │
         │ hello.iitk.ac.in ->     │
         │ 202.3.77.62             │
         └────────────┬────────────┘
                      │
                      ▼
         ┌─────────────────────────┐
         │ Print Execution Time    │
         │ (e.g., 0.213 sec)       │
         └────────────┬────────────┘
                      │
                      ▼
               ┌─────────────┐
               │    End      │
               └─────────────┘

### contributors 
Deepak chaurasia (220330),Readme 
Amruth Raj (220642),Code 