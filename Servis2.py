import string
from flask import Flask, request, jsonify
from collections import defaultdict
import datetime
import socket
import requests
import json

app = Flask(__name__)

SERVER_IP, SERVER_PORT = input("Введите IP-адрес и порт сервера СУБД (формат: myip:myport): ").split(':')
SERVER_PORT = int(SERVER_PORT)
statistics1 = defaultdict(list)
statistics2 = defaultdict(list)
statistics3 = defaultdict(list)
statistics4 = defaultdict(list)
statistics5 = defaultdict(list)
statistics6 = defaultdict(list)
    
def load():
    statistics1.clear()
    statistics2.clear()
    statistics3.clear()
    statistics4.clear()
    statistics5.clear()
    statistics6.clear()
    global count_tik
    count_tik = 0
    last_ip = ""
    last_url = ""
    while True:
        count_tik = count_tik + 1
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            try:
                s.connect((SERVER_IP, SERVER_PORT))
            except Exception as e:
                print (f'Не удалось подключиться к серверу: {str(e)}')
            s.sendall(f'--file urls.data --query HGET ip {count_tik}'.encode())
            ip = s.recv(1024).decode()
            ip = ip.replace('\n', '').replace('\r', '')
            if ip == "Error.":
                print ('Error.')
            if ip == "-> False":
                ip = last_ip
            last_ip = ip
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            try:
                s.connect((SERVER_IP, SERVER_PORT))
            except Exception as e:
                print (f'Не удалось подключиться к серверу: {str(e)}')
            s.sendall(f'--file urls.data --query HGET url {count_tik}'.encode())
            url = s.recv(1024).decode()
            url = url.replace('\n', '').replace('\r', '')
            if url == "Error.":
                print ('Error.')
            if url == "-> False":
                url = last_url
            last_url = url
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            try:
                s.connect((SERVER_IP, SERVER_PORT))
            except Exception as e:
                print (f'Не удалось подключиться к серверу: {str(e)}')
            s.sendall(f'--file urls.data --query HGET timestamp {count_tik}'.encode())
            timestamp = s.recv(1024).decode()
            timestamp = timestamp.replace('\n', '').replace('\r', '')
            if timestamp == "Error.":
                print ('Error.')
            if timestamp == "-> False":
                break
        statistics1[url].append((ip, timestamp))
        statistics2[url].append((timestamp, ip))
        statistics3[ip].append((url, timestamp))
        statistics4[ip].append((timestamp, url))
        statistics5[timestamp].append((ip, url))
        statistics6[timestamp].append((url, ip))

@app.route('/', methods=['POST'])
def post_statistics():
    load()
    ip = request.json['ip']
    url = request.json['url']
    timestamp = request.json['timestamp']
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        try:
            s.connect((SERVER_IP, SERVER_PORT))
        except Exception as e:
            return f'Не удалось подключиться к серверу: {str(e)}', 500
        s.sendall(f'--file urls.data --query HSET ip {ip} {count_tik}'.encode())
        if s.recv(1024).decode().replace('\n', '').replace('\r', '') == "Error.":
            return 'Error.', 500
        s.close()
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        try:
            s.connect((SERVER_IP, SERVER_PORT))
        except Exception as e:
            return f'Не удалось подключиться к серверу: {str(e)}', 500
        s.sendall(f'--file urls.data --query HSET url {url} {count_tik}'.encode())
        if s.recv(1024).decode().replace('\n', '').replace('\r', '') == "Error.":
            return 'Error.', 500
        s.close()
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        try:
            s.connect((SERVER_IP, SERVER_PORT))
        except Exception as e:
            return f'Не удалось подключиться к серверу: {str(e)}', 500
        s.sendall(f'--file urls.data --query HSET timestamp {timestamp} {count_tik}'.encode())
        if s.recv(1024).decode().replace('\n', '').replace('\r', '') == "Error.":
            return 'Error.', 500
        s.close()
    statistics1[url].append((ip, timestamp))
    statistics2[url].append((timestamp, ip))
    statistics3[ip].append((url, timestamp))
    statistics4[ip].append((timestamp, url))
    statistics5[timestamp].append((ip, url))
    statistics6[timestamp].append((url, ip))
    return '', 204

@app.route('/report', methods=['POST'])
def report():
    load()
    dimensions = request.json['Dimensions']
    report = []
    id = 1
    if (dimensions[0]=='URL' and dimensions[1]=='SourceIP'):
        for url, stats in statistics1.items():
            url_dict = {}
            for stat in stats:
                ip, timestamp = stat
                if ip not in url_dict:
                    url_dict[ip] = {'count': 1, 'timestamps': {timestamp: 1}}
                else:
                    if timestamp not in url_dict[ip]['timestamps']:
                        url_dict[ip]['timestamps'][timestamp] = 1
                    else:
                        url_dict[ip]['timestamps'][timestamp] += 1
            total_count = sum(sum(data['timestamps'].values()) for data in url_dict.values())
            report.append({
                'Id': id,
                'Pid': None,
                'URL': url,
                'SourceIP': None,
                'TimeInterval': None,
                'Count': total_count
            })
            id += 1
            for ip, data in url_dict.items():
                if 'SourceIP' in dimensions:
                    report.append({
                        'Id': id,
                        'Pid': 1,
                        'URL': None,
                        'SourceIP': ip,
                        'TimeInterval': None,
                        'Count': sum(data['timestamps'].values())
                    })
                    id += 1
                for timestamp, timestamp_count in data['timestamps'].items():
                    if 'TimeInterval' in dimensions:
                        report.append({
                            'Id': id,
                            'Pid': 2,
                            'URL': None,
                            'SourceIP': None,
                            'TimeInterval': str(timestamp),
                            'Count': timestamp_count
                        })
                        id += 1
    if (dimensions[0]=='URL' and dimensions[1]=='TimeInterval'):
        for url, stats in statistics2.items():
            url_dict = {}
            for stat in stats:
                timestamp, ip = stat
                if timestamp not in url_dict:
                    url_dict[timestamp] = {'count': 1, 'ips': {ip: 1}}
                else:
                    if ip not in url_dict[timestamp]['ips']:
                        url_dict[timestamp]['ips'][ip] = 1
                    else:
                        url_dict[timestamp]['ips'][ip] += 1
            total_count = sum(sum(data['ips'].values()) for data in url_dict.values())
            report.append({
                'Id': id,
                'Pid': None,
                'URL': url,
                'SourceIP': None,
                'TimeInterval': None,
                'Count': total_count
            })
            id += 1
            for timestamp, data in url_dict.items():
                if 'TimeInterval' in dimensions:
                    report.append({
                        'Id': id,
                        'Pid': 1,
                        'URL': None,
                        'SourceIP': None,
                        'TimeInterval': str(timestamp),
                        'Count': sum(data['ips'].values())
                    })
                    id += 1
                for ip, ip_count in data['ips'].items():
                    if 'SourceIP' in dimensions:
                        report.append({
                            'Id': id,
                            'Pid': 2,
                            'URL': None,
                            'SourceIP': ip,
                            'TimeInterval': None,
                            'Count': ip_count
                        })
                        id += 1
    if (dimensions[0]=='SourceIP' and dimensions[1]=='URL'):
        for ip, stats in statistics3.items():
            ip_dict = {}
            for stat in stats:
                url, timestamp = stat
                if url not in ip_dict:
                    ip_dict[url] = {'count': 1, 'timestamps': {timestamp: 1}}
                else:
                    if timestamp not in ip_dict[url]['timestamps']:
                        ip_dict[url]['timestamps'][timestamp] = 1
                    else:
                        ip_dict[url]['timestamps'][timestamp] += 1
            total_count = sum(sum(data['timestamps'].values()) for data in ip_dict.values())
            report.append({
                'Id': id,
                'Pid': None,
                'URL': None,
                'SourceIP': ip,
                'TimeInterval': None,
                'Count': total_count
            })
            id += 1
            for url, data in ip_dict.items():
                if 'URL' in dimensions:
                    report.append({
                        'Id': id,
                        'Pid': 1,
                        'URL': url,
                        'SourceIP': None,
                        'TimeInterval': None,
                        'Count': sum(data['timestamps'].values())
                    })
                    id += 1
                for timestamp, timestamp_count in data['timestamps'].items():
                    if 'TimeInterval' in dimensions:
                        report.append({
                            'Id': id,
                            'Pid': 2,
                            'URL': None,
                            'SourceIP': None,
                            'TimeInterval': str(timestamp),
                            'Count': timestamp_count
                        })
                        id += 1
    if (dimensions[0]=='SourceIP' and dimensions[1]=='TimeInterval'):
        for ip, stats in statistics4.items():
            report.append({
                'Id': id,
                'Pid': None,
                'URL': None,
                'SourceIP': ip,
                'TimeInterval': None,
                'Count': len(stats)
            })
            id += 1
            timestamp_dict = {}
            for stat in stats:
                timestamp, url = stat
                if timestamp not in timestamp_dict:
                    timestamp_dict[timestamp] = {'count': 1, 'urls': {url: 1}}
                else:
                    timestamp_dict[timestamp]['count'] += 1
                    if url not in timestamp_dict[timestamp]['urls']:
                        timestamp_dict[timestamp]['urls'][url] = 1
                    else:
                        timestamp_dict[timestamp]['urls'][url] += 1
            for timestamp, data in timestamp_dict.items():
                if 'TimeInterval' in dimensions:
                    report.append({
                        'Id': id,
                        'Pid': 1,
                        'URL': None,
                        'SourceIP': None,
                        'TimeInterval': str(timestamp),
                        'Count': data['count']
                    })
                    id += 1
                for url, url_count in data['urls'].items():
                    if 'URL' in dimensions:
                        report.append({
                            'Id': id,
                            'Pid': 2,
                            'URL': url,
                            'SourceIP': None,
                            'TimeInterval': None,
                            'Count': url_count
                        })
                        id += 1
    if (dimensions[0]=='TimeInterval' and dimensions[1]=='SourceIP'):
        for timestamp, stats in statistics5.items():
            report.append({
                'Id': id,
                'Pid': None,
                'URL': None,
                'SourceIP': None,
                'TimeInterval': str(timestamp),
                'Count': len(stats)
            })
            id += 1
            ip_dict = {}
            for stat in stats:
                ip, url = stat
                if ip not in ip_dict:
                    ip_dict[ip] = {'count': 1, 'urls': {url: 1}}
                else:
                    if url not in ip_dict[ip]['urls']:
                        ip_dict[ip]['count'] += 1
                        ip_dict[ip]['urls'][url] = 1
                    else:
                        ip_dict[ip]['urls'][url] += 1
            for ip, data in ip_dict.items():
                if 'SourceIP' in dimensions:
                    report.append({
                        'Id': id,
                        'Pid': 1,
                        'URL': None,
                        'SourceIP': ip,
                        'TimeInterval': None,
                        'Count': sum(data['urls'].values())
                    })
                    id += 1
                for url, url_count in data['urls'].items():
                    if 'URL' in dimensions:
                        report.append({
                            'Id': id,
                            'Pid': 2,
                            'URL': url,
                            'SourceIP': None,
                            'TimeInterval': None,
                            'Count': url_count
                        })
                        id += 1
    if (dimensions[0]=='TimeInterval' and dimensions[1]=='URL'):
        for timestamp, stats in statistics6.items():
            report.append({
                'Id': id,
                'Pid': None,
                'URL': None,
                'SourceIP': None,
                'TimeInterval': str(timestamp),
                'Count': len(stats)
            })
            id += 1
            url_dict = {}
            for stat in stats:
                url, ip = stat
                if url not in url_dict:
                    url_dict[url] = {'count': 1, 'ips': {ip: 1}}
                else:
                    if ip not in url_dict[url]['ips']:
                        url_dict[url]['ips'][ip] = 1
                    else:
                        url_dict[url]['ips'][ip] += 1
                    url_dict[url]['count'] = sum(url_dict[url]['ips'].values())
            for url, data in url_dict.items():
                if 'URL' in dimensions:
                    report.append({
                        'Id': id,
                        'Pid': 1,
                        'URL': url,
                        'SourceIP': None,
                        'TimeInterval': None,
                        'Count': data['count']
                    })
                    id += 1
                for ip, ip_count in data['ips'].items():
                    if 'SourceIP' in dimensions:
                        report.append({
                            'Id': id,
                            'Pid': 2,
                            'URL': None,
                            'SourceIP': ip,
                            'TimeInterval': None,
                            'Count': ip_count
                        })
                        id += 1
    formatted_report = format_report(report, dimensions)
    return jsonify(formatted_report)

def format_report(report, dimensions):
    max_length = max(len(item['URL']) for item in report if item['URL']) if 'URL' in dimensions else 0
    max_length = max(max_length, max(len(item['SourceIP']) for item in report if item['SourceIP'])) if 'SourceIP' in dimensions else max_length
    max_length = max(max_length, max(len(item['TimeInterval']) for item in report if item['TimeInterval'])) if 'TimeInterval' in dimensions else max_length
    formatted_report = ["\t\tДетализация".ljust(max_length) + "Количество переходов"]
    for item in report:
        line = "\t" * item['Pid'] if item['Pid'] is not None else ""
        for dimension in dimensions:
            if dimension == 'URL' and item['URL']:
                line += f"{item['URL'].ljust(max_length)}\t"
            elif dimension == 'SourceIP' and item['SourceIP']:
                line += f"{item['SourceIP'].ljust(max_length)}\t"
            elif dimension == 'TimeInterval' and item['TimeInterval']:
                line += f"{item['TimeInterval'].ljust(max_length)}\t"
        if item['Pid'] == None:
            line += "\t" * 2
        elif item['Pid'] == 1:
            line += "\t" * 1
        elif item['Pid'] == 2:
            line += ""
        line += f"{item['Count']}"
        formatted_report.append(line)
    return '<pre>' + '\n'.join(formatted_report) + '</pre>'

@app.route('/get_report', methods=['GET'])
def get_report():
    dimensions = request.args.get('dimensions').split(',')
    response = requests.post('http://localhost:5001/report', json={"Dimensions": dimensions})
    return response.json()

if __name__ == '__main__':
    app.run(port=5001, debug=True)
