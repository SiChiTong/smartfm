package com.leon.golfcar;

import java.util.Enumeration;
import java.util.List;
import com.google.android.maps.GeoPoint;
import com.google.android.maps.MapActivity;
import com.google.android.maps.MapController;
import com.google.android.maps.MapView;
import com.google.android.maps.Overlay;
import com.leon.golfcar.R;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Point;
import android.location.Geocoder;
import android.location.Location;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.widget.Toast;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketException;
import java.util.ArrayList;


public class MainActivity extends MapActivity  {
    /** Called when the activity is first created. */
    private MapView mapView = null;
    private MapController mapController = null;
	private GeoPoint destGeoPoint = null;
	private GeoPoint pickupGeoPoint;
    private double destLatitude = 0.0;
    private double destLongitude = 0.0;
    private double pickupLatitude = 0.0;
    private double pickupLongitude = 0.0;
    private boolean orderFlag = false;
    private int userID;
    private int taskID = 1;
    
    final private int REQUEST_CODE = 1;
    
    private static final String REMOTE_HOSTNAME = "172.17.185.120";
    // Phone's IP and Port for sending and listening
    public static String LOCAL_SERVERIP = "172.17.184.92"; //default
    public static final int LOCAL_SERVERPORT = 4440;
    
    private ServerSocket serverSocket;
	
    //private FMUI client = new FMUI();
    private List<ServiceMsg> userOrders = new ArrayList<ServiceMsg>();
    
	@Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
		setTitle("Google Map");
        // set up GoogleMap
		mapView = (MapView) findViewById(R.id.mapView);
        mapView.setSatellite(true);
        mapView.setEnabled(true);
        mapView.setClickable(true);
        mapView.setBuiltInZoomControls(true);
        mapView.displayZoomControls(true);
        mapController = mapView.getController();
        
        // Get Phone's local Ip address
        LOCAL_SERVERIP = getLocalIpAddress();
        
        // Generate an unique ID for this Phone
        userID = generateUserID(LOCAL_SERVERIP);
        
        // Create a Thread for network information transfer
        Thread fst = new Thread(new ListeningThread());
        fst.setName("Thread:onCreate");
        fst.start();
    }
	
	// Generate ID
	private int generateUserID(String SERVERIP){
		return SERVERIP.hashCode() > 0 ? SERVERIP.hashCode() : -SERVERIP.hashCode();
	}
	

	// check connection to Server
	private void checkConnectionWithServer(){
		 //if (!client.connection) {
	       if (true){
			Context context = getApplicationContext();
	        	CharSequence text = "Cannot connection to Server. Please check Internet settings";
	        	int duration = Toast.LENGTH_LONG;
	        	
	        	Toast toast = Toast.makeText(context, text, duration);
	        	toast.show();
	        }
	}

	
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		MenuInflater inflater = getMenuInflater();
		inflater.inflate(R.menu.service, menu);
		return true;
	}
	
	@Override
	public boolean onPrepareOptionsMenu(Menu menu) {
		MenuItem setting = menu.findItem(R.id.check_setting);
		CharSequence on = "Connection: ON";
		CharSequence off = "Connection: OFF";
		
		//if(client.connection)
			setting.setTitle(on);
		//else
		//	setting.setTitle(off);
		
		return super.onPrepareOptionsMenu(menu);
	}
	
	//define Option Menu, including set_pickup, set_destination, get_status, cancel task
	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		switch (item.getItemId()){
		case R.id.book_car:
			Intent intent_bookCar = new Intent(MainActivity.this,BookCarActivity.class);
			startActivityForResult(intent_bookCar, REQUEST_CODE);
			return true;
		case R.id.service_info:
			Intent intent_serviceInfo = new Intent(MainActivity.this,ServiceInfoActivity2.class);		
			ArrayList<String> tmp = new ArrayList<String>();
			// store tasks info into tmp
			for (int i=0;i<userOrders.size();i++)
			{
				tmp.add(userOrders.get(i).ID+":"+userOrders.get(i).taskID+":"+userOrders.get(i).pickup_op+":"+userOrders.get(i).dest_op
						+":"+userOrders.get(i).wait+":"+userOrders.get(i).carID+":"+userOrders.get(i).pickup_location+":"
						+userOrders.get(i).dest_location);
			}
			intent_serviceInfo.putStringArrayListExtra("tmp", tmp);
			startActivity(intent_serviceInfo);
			return true;
		case R.id.cancel_service:
			cancelOrder();
			return true;	
		default:
			return super.onOptionsItemSelected(item);
		}
	}
	
	//cancel order
	private void cancelOrder() {
		//clear service	
		//pickup_op = -1;
		//dest_op = -1;
	    destLatitude = 0.0;
	    destLongitude = 0.0;
	    pickupLatitude = 0.0;
	    pickupLongitude = 0.0;
	    //dest_location = null;
	    //pickup_location = null;
		//wait = "-1";
		//carID = "-1";
		
		//clear overlay
		List<Overlay> list = mapView.getOverlays();
		list.clear();
		
		Intent intentOfCancel = new Intent(MainActivity.this, CancelActivity.class);
		startActivity(intentOfCancel);
		
		//send pickup location to Server
		//String[] order = {"client","-i",hostname,userid,Integer.toString(pickup_op),Integer.toString(dest_op)};
		//client.main_client(order);		
		
		
		orderFlag = true;
		
		//checkInternet();
		
	}
	
	//handle return value from RadioGroupActivity and RadioGroupActivity
	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {		
		if (requestCode == REQUEST_CODE) {
			if (resultCode == RESULT_OK){
	    		Bundle extras = data.getExtras();
				if (extras != null)
				{
					//ServiceMsg 
					//get data from BookCarActivity.java
					pickupLatitude = extras.getDouble("pickup_lat");
					pickupLongitude = extras.getDouble("pickup_longi");
					destLatitude = extras.getDouble("dest_lat");
					destLongitude = extras.getDouble("dest_longi");	
					//draw dest location
					//convert destination GeoPoint to location name
					Location dest_location = new Location("dummyprovider");
					dest_location.setLatitude(destLatitude);
					dest_location.setLongitude(destLongitude);
					// draw destination pin on Map
					destGeoPoint = new GeoPoint ((int)(destLatitude * 1000000), (int)(destLongitude * 1000000));        
					mapController.animateTo(destGeoPoint);
					mapController.setZoom(17);	
					drawDestinationLocationOverlay dest_overlay = new drawDestinationLocationOverlay();
					List<Overlay> dest_list = mapView.getOverlays();
					dest_list.add(dest_overlay);
					
					// draw pickup location
					//convert pickup GeoPoint to location name
					Location location = new Location("pickup");
					location.setLatitude(pickupLatitude);
					location.setLongitude(pickupLongitude);
					
					// draw pickup location pin on Map
					pickupGeoPoint = new GeoPoint ((int)(pickupLatitude * 1000000), (int)(pickupLongitude * 1000000));        
					mapController.animateTo(pickupGeoPoint);
					mapController.setZoom(17);	
					
					drawPickupLocationOverlay pickup_overlay = new drawPickupLocationOverlay();
					List<Overlay> pickup_list = mapView.getOverlays();
					pickup_list.add(pickup_overlay);
					
					// add new task to services 
					ServiceMsg s = new ServiceMsg(Integer.toString(userID),Integer.toString(taskID++),Integer.toString(extras.getInt("pickup_op")),
							Integer.toString(extras.getInt("dest_op")),"0","0",extras.getString("pickup_location"),extras.getString("dest_location"));
					userOrders.add(s);
					
					orderFlag = true;
				}
			}
		}
	}
	
	//draw pickup location overlay
	public class drawPickupLocationOverlay extends Overlay
	{
		public  GeoPoint gp0=null;
		public  GeoPoint gp1=null;
		
		public boolean draw(Canvas canvas, MapView mapView, boolean shadow, long when)
		{
			super.draw(canvas, mapView, shadow);
			Paint paint = new Paint();
			Point myScreenCoords = new Point();
			Bitmap bmp;
			
			//convert
			mapView.getProjection().toPixels(pickupGeoPoint, myScreenCoords);
			paint.setStrokeWidth(1);
			paint.setARGB(255, 255, 0, 0);
			paint.setStyle(Paint.Style.STROKE);
			bmp = BitmapFactory.decodeResource(getResources(),R.drawable.car);
			canvas.drawBitmap(bmp, myScreenCoords.x, myScreenCoords.y,paint);
			
			return true;	
		}
		
		public void disableCompass() {
			// TODO Auto-generated method stub
			
		}
		
		public void enableCompass() {
			// TODO Auto-generated method stub
			
		}
	}
	
	//draw destination location overlay
	public class drawDestinationLocationOverlay extends Overlay
	{
		public  Geocoder geocoder;
		public  GeoPoint gp0=null;
		public  GeoPoint gp1=null;
		
		public boolean draw(Canvas canvas, MapView mapView, boolean shadow, long when)
		{
			super.draw(canvas, mapView, shadow);
			Paint paint = new Paint();
			Point myScreenCoords = new Point();
			Bitmap bmp;
			//convert
			
			mapView.getProjection().toPixels(destGeoPoint, myScreenCoords);
			paint.setStrokeWidth(1);
			paint.setARGB(255, 255, 0, 0);
			paint.setStyle(Paint.Style.STROKE);
			
			bmp = BitmapFactory.decodeResource(getResources(),R.drawable.cabin);			
			canvas.drawBitmap(bmp, myScreenCoords.x, myScreenCoords.y,paint);
			
			return true;
			
		}
		
		public void disableCompass() {
			// TODO Auto-generated method stub
			
		}
		
		public void enableCompass() {
			// TODO Auto-generated method stub
			
		}
	}
	
    public class ListeningThread implements Runnable {
		
        public void run() {
            try {
                if (LOCAL_SERVERIP != null) {
                	
                	int[] ports = {4440, 4441, 1024, 8080};
                	PrintWriter out = null;
                    BufferedReader in = null;
                    
                    serverSocket = new ServerSocket(LOCAL_SERVERPORT);
                    Socket socket = null;
                    
                    for( int p: ports ) {
        				socket = new Socket(REMOTE_HOSTNAME,p);
        				out = new PrintWriter(socket.getOutputStream(), true);
        			    in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
        			    if (socket!=null) {
        	                break;
        	            }
                    }
                    
                	while(true){
                		if (orderFlag) {
                			out.println(";" + userOrders.get(userOrders.size()-1).ID + ":" 
                			+ userOrders.get(userOrders.size()-1).taskID + ":" 
                			+ userOrders.get(userOrders.size()-1).pickup_op + ":" 
                			+ userOrders.get(userOrders.size()-1).dest_op);
                			orderFlag = false;
                		} 
                		
                		try {
                            String msg = null;
                            
                            //;task_id:wait:carID
                            while (in.ready()&&(msg = in.readLine()) != null) {                                
                                if (msg.startsWith(";")) {
                                	int f = msg.indexOf(":");
                                	int s = msg.indexOf(":",f+1);
                                	String temp_taskID = msg.substring(1,f);		
                                	
                                	// check taskID
                                	int sidx = -1;
                                	for (int i=0;i<userOrders.size();i++)
                                	{
                                		if(userOrders.get(i).taskID.compareTo(temp_taskID)==0){
                                			sidx = i;
                                			break;
                                		}		
                                	}
                                	
                                	if(sidx==-1){
                                		System.out.println("No TaskID: "+temp_taskID);
                                	}
                                	else{
                                		userOrders.get(sidx).wait = msg.substring(f+1, s);
                                		userOrders.get(sidx).carID = msg.substring(s+1);
                                	}         
                                    break;
                                }
                            }
                            //break;
                        } catch (Exception e) {
                        	e.printStackTrace();
                        }                    
                	
                		try {
                            Thread.sleep(1000);
                        } catch( InterruptedException e ) {
                        }
                	}
                	
                	
                    
                    
                } else {	//ServiceIP = null
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }
	
    // gets the ip address of your phone's network
    private String getLocalIpAddress() {
        try {
            for (Enumeration<NetworkInterface> en = NetworkInterface.getNetworkInterfaces(); en.hasMoreElements();) {
                NetworkInterface intf = en.nextElement();
                for (Enumeration<InetAddress> enumIpAddr = intf.getInetAddresses(); enumIpAddr.hasMoreElements();) {
                    InetAddress inetAddress = enumIpAddr.nextElement();
                    if (!inetAddress.isLoopbackAddress()) { return inetAddress.getHostAddress().toString(); }
                }
            }
        } catch (SocketException ex) {
            Log.e("ServerActivity", ex.toString());
        }
        return null;
    }
	
    @Override
    protected void onStop() {
        super.onStop();
        try {
			// make sure you close the socket upon exiting
			serverSocket.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
    }

	@Override
	protected boolean isRouteDisplayed() {
		// TODO Auto-generated method stub
		return false;
	}
}


