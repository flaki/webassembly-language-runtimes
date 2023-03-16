import sdk
import re

def run_e(payload, ident):
    # Request the contents of the Suborbital team page
    response = sdk.http_get("https://suborbital.dev/team", ident)

    # If payload is defined, use it as the name
    who = payload if payload != "" else 'Flaki'

    # Try to find the given name on the page and extract their role
    # Skip the embedded <style>-s present in some of the elements
    regex = ">"+who+"</span>(?:<style[^>]*>.*?</style>)?[^>]+>([^<]+)"
    match = re.search(regex, response.decode("utf-8"))

    # Use the matched role, decode any HTML entities
    what = match.group(1).replace("&amp;","&") if match != None else "?"

    # Log the name and role into the log and return the role in the result
    sdk.log_info(f"{who} at suborbital.dev is a(n) '{what}'", ident)
    sdk.return_result(what, ident)
