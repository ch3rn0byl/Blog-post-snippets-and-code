import argparse
import sys
import random
try:
    from bs4 import BeautifulSoup
    import requests
except ModuleNotFoundError as e:
    print('[!] {}'.format(e))
    print('[!] pip install <package>')
    sys.exit(-1)

# Delete the below three lines if you have Python 2 only
if sys.version_info.major != 3:
    print('[!] This needs to be ran under Python 3')
    print('[!] python3 {} -k keywords'.format(sys.argv[0]))
    sys.exit(1)

def getCases():
    url = b'https://www.justice.gov/civil/current-and-recent-cases#_Decep'

    useragents = [
        b'Mozilla/5.0 (Macintosh; Intel Mac OS X 10.14; rv:10.0) Gecko/20100101 Firefox/62.0',
        b'Mozilla/5.0 (X11; Linux i686; rv:64.0) Gecko/20100101 Firefox/64.0',
        b'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML like Gecko) Chrome/51.0.2704.79 Safari/537.36 Edge/14.14931',
        b'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/70.0.3538.77 Safari/537.36',
    ]

    headers = {
        'User-Agent' : random.choice(useragents)
    }

    r = requests.get(url, headers=headers)
    return r

def main(arguments):
    keywords = arguments.k

    data = getCases()
    soup = BeautifulSoup(data.content, 'html.parser')
    soup = soup.find_all('tbody')[-1]
    soup = soup.find_all('p')

    '''
    This part will take out the useless part dynamically, since I'm not sure if its 
    always hardcoded at offset -2, then removes it #YOLO
    '''
    index = 0
    while True:
        if 'CLOSED CASES' in soup[index].text.strip():
            del soup[index]
            break
        index -= 1

    '''
    These items can be divided into a summary, court case, press release if applicable
    docket number, and what happened
    The very last item after each section is ended with <p> </p>. The "space" between 
    that is translated to 0xa0, which is probably a typo because 0x0a is a line feed
    
    FOR NOW
    '''
    hits = 0
    fullcase = ''
    for line in soup:
        if len(line.text) == 1 and ord(line.text) == 160:
            for keyword in keywords:
                if keyword.lower() in fullcase:
                    print('[+] Got a hit!')
                    print(fullcase)
                    hits += 1
                    break
            fullcase = ''
        else:
            fullcase += '{}\n'.format(line.text.lower().strip())
    if hits == 0:
        print('[+] No hits were found with: {}'.format(keywords))
    elif hits == 1:
        print('[+] Only got 1 hit!')
    else:
        print('[+] We got a total of {} hits!'.format(hits))

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Tell your these/PhD thingy to suck it!',
        epilog='>:)'
    )
    parser.add_argument('-k', metavar='keyword', nargs='+', required=True, help='Search for a keyword')
    args = parser.parse_args()
    main(args)
