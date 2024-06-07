import boto3
import csv

def lambda_handler(event, context):
    s3 = boto3.client('s3')
    ses = boto3.client('ses')
    bucket_name = event['Records'][0]['s3']['bucket']['name']
    key = event['Records'][0]['s3']['object']['key']
    response = s3.get_object(Bucket=bucket_name, Key=key)
    lines = response['Body'].read().decode('utf-8').split()
    for row in csv.DictReader(lines):
        ses.send_email(
            Source='your-verified-email@example.com',
            Destination={'ToAddresses': [row['email']]},
            Message={
                'Subject': {'Data': 'Your Email Subject'},
                'Body': {'Text': {'Data': "Your email content"}}
            }
        )
