api_token = 'nconafihztsullnskpbajnufsrrtkruw'
api_url = 'https://codeweekend.dev:3723/api/'
files_url = 'https://codeweekend.dev:81/'

import requests
import json
import time
import os
import sys

headers = {
    'authorization': f'Bearer {api_token}'
}

def show(inner_json):
    print(json.dumps(inner_json, indent=2))

def get_scoreboard():
    return requests.get(api_url + 'scoreboard', headers=headers).json()

def get_team_dashboard():
    return requests.get(api_url + 'team_dashboard', headers=headers).json()

def get_test(task_id):
    task_id_padded = '{:03d}'.format(task_id)
    url = f'{files_url}{task_id_padded}.json'
    return requests.get(url, headers=headers).content

# Returns at most 50 submissions
def get_team_submissions(offset=0, task_id=None):
    url = f'{api_url}team_submissions?offset={offset}'
    if task_id is not None:
        url += f'&task_id={task_id}'
    return requests.get(url, headers=headers).json()

def get_submission_info(submission_id, wait=False):
    url = f'{api_url}submission_info/{submission_id}'
    res = requests.get(url, headers=headers).json()
    if 'Pending' in res and wait:
        print('Submission is in Pending state, waiting...')
        time.sleep(1)
        return get_submission_info(submission_id)
    return res

# Returns submission_id
def submit(task_id, solution):
    res = requests.post(url = f'{api_url}submit/{task_id}',
                        headers=headers, files={'file': solution})
    if res.status_code == 200:
        return res.text
    print(f'Error: {res.text}')
    return None

def download_submission(submission_id):
    import urllib.request
    url = f'{api_url}download_submission/{submission_id}'
    opener = urllib.request.build_opener()
    opener.addheaders = headers.items()
    urllib.request.install_opener(opener)
    try:
        file, _ = urllib.request.urlretrieve(url, "downloaded.txt")
    except Exception as e:
        print('Failed to download submission: ', e)
        return None
    content = open(file, "r").read()
    os.remove(file)
    return content

def update_display_name(new_name):
    url = api_url + 'update_user'
    data = {
        'display_name': new_name,
        'email': "",
        'team_members': ""
    }
    return requests.post(url, json=data, headers=headers).content

# show(get_scoreboard())
# show(get_submission_info(427))
# show(get_team_dashboard())
# show(get_team_submissions())
# download_submission(476)
# get_test(1)
#update_display_name('Test 123')

# Submission example
'''def example():
    submission_id = submit(1, '{ "a" : 123 }')
    print(f'Submission_id: {submission_id}')
    info = get_submission_info(submission_id, wait=True)
    print(f'Result: {info}')

example()'''

test_id = sys.argv[1]
filename = "Tests/test_" + sys.argv[1] + "_answer"
f = open(filename, "r")
f.readline() # trim "score": X
file_contents = f.read()
s = submit(test_id, file_contents)
if s != None:
    print(s)