from collections import OrderedDict
from datetime import date, datetime, timedelta

from flask import Flask, jsonify, abort, render_template, url_for
import requests

URL = "https://www.gov.uk/bank-holidays.json"
DATE_FORMAT = "%Y-%m-%d"  # "2018-08-27"

HOLIDAY_DATA = None
LAST_UPDATE = None
MAX_AGE = timedelta(hours=24)

app = Flask(__name__, subdomain_matching=True)
app.config['SERVER_NAME'] = "doineedbunting.today"


@app.route("/")
def home():
    data = get_holiday_data()
    str_today = date_today()
    regions = OrderedDict()
    for region in data.keys():
        regions[region] = _does_day_need_bunting(data[region], str_today)
        regions[region].update({'region': _humaise_region_name(region)})

    regions = OrderedDict(sorted(regions.items(), key=lambda t: t[0]))  # sort for consistency
    return render_template('index.html', regions=regions)


@app.route("/<region>", subdomain='api')
def region_bunting_api(region):
    data = get_holiday_data()
    return jsonify(does_day_need_bunting(data, region))


@app.route("/<region>")
def region_bunting(region):
    data = get_holiday_data()
    res = does_day_need_bunting(data, region)
    return render_template('region.html', data=res)


@app.route("/<region>/<day>")
def day_bunting(region, day):
    data = get_holiday_data()
    res = does_day_need_bunting(data, region, day)
    return render_template('region.html', data=res)


@app.route("/<template>.html")
def why(template):
    return render_template('%s.html' % template)


def does_day_need_bunting(data, region, day=None):
    day = date_today() if day is None else day
    if region not in data.keys():
        abort(404, "Region '%s' not found, please try again" % region)

    res = _does_day_need_bunting(data[region], day)
    res.update({'region': _humaise_region_name(region)})  # add readable region name for rendering. skip this bit for api.

    return res


def _does_day_need_bunting(region_data, day):
    for event in region_data['events']:
        if event['date'] == day:
            return event
    return {'bunting': False}


def _humaise_region_name(region_name):
    return region_name.replace('-', ' ').title().replace('And', 'and')


def refresh_holiday_data():
    global HOLIDAY_DATA
    HOLIDAY_DATA = requests.get(URL).json()
    global LAST_UPDATE
    LAST_UPDATE = datetime.now()


def get_holiday_data():
    if HOLIDAY_DATA is None or LAST_UPDATE < (datetime.now() - MAX_AGE):
        refresh_holiday_data()
    return HOLIDAY_DATA


def date_today():
    # return "2019-12-26"
    return datetime.strftime(date.today(), DATE_FORMAT)
