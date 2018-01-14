import boto3
import json

from shapely.geometry import Polygon
from shapely.geometry import Point

dynamodb = boto3.resource('dynamodb')
wranglebase = dynamodb.Table('wranglebase')
sns = boto3.client('sns')

# TODO: fetch according to user id
fence = [
    [-110.959502, 32.236016],
    [-110.943994, 32.236070],
    [-110.943874, 32.227844],
    [-110.959462, 32.227734],
    [-110.959502, 32.236016],
]

# TODO: fetch according to user id
phoneNumber = '+1**********'

def does_fence_contain(lat, lng):
    polygon = Polygon(fence)
    point = Point((lng, lat))
    return polygon.contains(point)

def update_cow(id_, lat, lng, active):
    wranglebase.update_item(
        Key={
            'id': id_
        },
        UpdateExpression='SET #location = :loc, #active = :act',
        ExpressionAttributeNames={ "#location": "location", "#active": "active" },
        ExpressionAttributeValues={
            ':loc': f'{{\"type\":\"Point\",\"coordinates\":[{lng},{lat}]}}',
            ':act': active,
        }
    )

def get_cow(id_):
    return wranglebase.get_item(
        Key={
            'id': id_,
        }
    )['Item']

def handler(event, context):

    id_ = event.get('id', None)
    lat = event.get('lat', None)
    lng = event.get('lng', None)

    ### BEGIN VARIABLE CHECKS ###

    # make sure we know what cow it is.
    if not id_:
        raise Exception('[BadRequest] A cow is required.')

    # make sure we have a location.
    if not lat or not lng:
        raise Exception('[BadRequest] A latitude and longitude are required.')

    # make sure the location is a number.
    try:
        lat = float(lat)
        lng = float(lng)
    except TypeError:
        raise Exception('[BadRequest] Latitude and longitude must be numbers.')

    ### END VARIABLE CHECKS ###

    # get the current state of the cow.
    cow = get_cow(id_)
    active = does_fence_contain(lat, lng)

    # update the cow in the database.
    update_cow(id_, lat, lng, active)

    # cow was active, but now its not. send a notification.
    if cow['active'] and not active:
        message =  'A {} is escaping! Catch {} at {}, {}!'.format(cow['type'], cow['name'], lat, lng)
        print('sending a message', phoneNumber, message)
        sns.publish(PhoneNumber=phoneNumber, Message=message)

    # awww shieett
    return { 'message': 'awww shieett ya cow just moved *ding*ding*' }
