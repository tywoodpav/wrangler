import boto3
import json

dynamodb = boto3.resource('dynamodb')
wranglebase = dynamodb.Table('wranglebase')

def get_cows():
    for cow in wranglebase.scan()['Items']:
        yield {
            'id': cow['id'],
            'name': cow['name'],
            'type': cow['type'],
            'active': cow['active'],
            'location': json.loads(cow['location']),
        }

def handler(event, context):
    return { 'cows': list(get_cows()) }
