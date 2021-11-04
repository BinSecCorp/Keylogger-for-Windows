import requests
from bs4 import BeautifulSoup
page = requests.get("https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes")
soup = BeautifulSoup(page.content, 'lxml')
list_e = soup.find_all("tbody")[0]

textfile = open('C:/Users/Jason/Desktop/programs/keylogger/keylogger_ws/lut.txt', 'w')

for e in range(192):
    e2 = list_e.find_all('tr')[e]
    e3 = e2.find_all('td')[1]
    e4 = e2.find_all('dl')[0]
    str1 = e4.text
    
    textfile.write(f"{str1.split(' ')[2]} {e3.text}\n")
