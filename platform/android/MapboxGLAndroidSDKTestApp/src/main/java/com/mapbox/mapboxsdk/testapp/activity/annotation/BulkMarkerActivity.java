package com.mapbox.mapboxsdk.testapp.activity.annotation;

import android.animation.Animator;
import android.animation.AnimatorListenerAdapter;
import android.app.ProgressDialog;
import android.graphics.PorterDuff;
import android.graphics.drawable.Drawable;
import android.os.AsyncTask;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.v4.content.res.ResourcesCompat;
import android.support.v7.app.ActionBar;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import com.mapbox.mapboxsdk.annotations.Icon;
import com.mapbox.mapboxsdk.annotations.IconFactory;
import com.mapbox.mapboxsdk.annotations.Marker;
import com.mapbox.mapboxsdk.annotations.MarkerOptions;
import com.mapbox.mapboxsdk.annotations.MarkerViewOptions;
import com.mapbox.mapboxsdk.geometry.LatLng;
import com.mapbox.mapboxsdk.maps.MapView;
import com.mapbox.mapboxsdk.maps.MapboxMap;
import com.mapbox.mapboxsdk.maps.OnMapReadyCallback;
import com.mapbox.mapboxsdk.testapp.R;
import com.mapbox.mapboxsdk.testapp.utils.GeoParseUtil;

import org.json.JSONException;

import java.io.IOException;
import java.text.DecimalFormat;
import java.util.ArrayList;
import java.util.List;
import java.util.Random;

public class BulkMarkerActivity extends AppCompatActivity implements AdapterView.OnItemSelectedListener {

    private MapboxMap mapboxMap;
    private MapView mapView;
    private boolean customMarkerView;
    private List<LatLng> locations;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_marker_bulk);

        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        final ActionBar actionBar = getSupportActionBar();
        if (actionBar != null) {
            actionBar.setDisplayShowTitleEnabled(false);
            actionBar.setDisplayHomeAsUpEnabled(true);
            actionBar.setDisplayShowHomeEnabled(true);
        }

        mapView = (MapView) findViewById(R.id.mapView);
        mapView.onCreate(savedInstanceState);
        mapView.getMapAsync(new OnMapReadyCallback() {
            @Override
            public void onMapReady(@NonNull MapboxMap mapboxMap) {
                BulkMarkerActivity.this.mapboxMap = mapboxMap;

                if (actionBar != null) {
                    ArrayAdapter<CharSequence> spinnerAdapter = ArrayAdapter.createFromResource(
                        actionBar.getThemedContext(), R.array.bulk_marker_list, android.R.layout.simple_spinner_item);
                    spinnerAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
                    Spinner spinner = (Spinner) findViewById(R.id.spinner);
                    spinner.setAdapter(spinnerAdapter);
                    spinner.setOnItemSelectedListener(BulkMarkerActivity.this);
                }
            }
        });

        final View fab = findViewById(R.id.fab);
        if (fab != null) {
            fab.setOnClickListener(new FabClickListener());
        }
    }

    @Override
    public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
        int amount = Integer.valueOf(getResources().getStringArray(R.array.bulk_marker_list)[position]);
        if (locations == null) {
            new LoadLocationTask(this, amount).execute();
        } else {
            showMarkers(amount);
        }
    }

    private void onLatLngListLoaded(List<LatLng> latLngs, int amount) {
        locations = latLngs;
        showMarkers(amount);
    }

    private void showMarkers(int amount) {
        mapboxMap.clear();

        if (locations.size() < amount) {
            amount = locations.size();
        }

        if (customMarkerView) {
            showViewMarkers(amount);
        } else {
            showGlMarkers(amount);
        }
    }

    private void showViewMarkers(int amount) {
        DecimalFormat formatter = new DecimalFormat("#.#####");
        Random random = new Random();
        int randomIndex;

        Drawable drawable = ResourcesCompat.getDrawable(getResources(), R.drawable.ic_droppin_24dp, getTheme());
        int redColor = ResourcesCompat.getColor(getResources(), android.R.color.holo_red_dark, getTheme());
        drawable.setColorFilter(redColor, PorterDuff.Mode.SRC_IN);
        Icon icon = IconFactory.getInstance(this).fromDrawable(drawable);

        List<MarkerViewOptions> markerOptionsList = new ArrayList<>();
        for (int i = 0; i < amount; i++) {
            randomIndex = random.nextInt(locations.size());
            LatLng latLng = locations.get(randomIndex);
            MarkerViewOptions markerOptions = new MarkerViewOptions()
                .position(latLng)
                .icon(icon)
                .title(String.valueOf(i))
                .snippet(formatter.format(latLng.getLatitude()) + ", " + formatter.format(latLng.getLongitude()));
            markerOptionsList.add(markerOptions);
        }
        mapboxMap.addMarkerViews(markerOptionsList);
    }

    private void showGlMarkers(int amount) {
        List<MarkerOptions> markerOptionsList = new ArrayList<>();
        DecimalFormat formatter = new DecimalFormat("#.#####");
        Random random = new Random();
        int randomIndex;

        for (int i = 0; i < amount; i++) {
            randomIndex = random.nextInt(locations.size());
            LatLng latLng = locations.get(randomIndex);
            markerOptionsList.add(new MarkerOptions()
                .position(latLng)
                .title(String.valueOf(i))
                .snippet(formatter.format(latLng.getLatitude()) + ", " + formatter.format(latLng.getLongitude())));
        }

        mapboxMap.addMarkers(markerOptionsList);
    }

    @Override
    public void onNothingSelected(AdapterView<?> parent) {
        // nothing selected, nothing to do!
    }

    @Override
    public void onResume() {
        super.onResume();
        mapView.onResume();
    }

    @Override
    public void onPause() {
        super.onPause();
        mapView.onPause();
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        mapView.onSaveInstanceState(outState);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mapView.onDestroy();
    }

    @Override
    public void onLowMemory() {
        super.onLowMemory();
        mapView.onLowMemory();
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case android.R.id.home:
                onBackPressed();
                return true;
            default:
                return super.onOptionsItemSelected(item);
        }
    }

    private class FabClickListener implements View.OnClickListener {
        @Override
        public void onClick(final View v) {
            if (mapboxMap != null) {
                customMarkerView = true;

                // remove fab
                v.animate().alpha(0).setListener(new AnimatorListenerAdapter() {
                    @Override
                    public void onAnimationEnd(Animator animation) {
                        super.onAnimationEnd(animation);
                        v.setVisibility(View.GONE);
                    }
                }).start();

                // reload markers
                Spinner spinner = (Spinner) findViewById(R.id.spinner);
                if (spinner != null) {
                    int amount = Integer.valueOf(
                        getResources().getStringArray(R.array.bulk_marker_list)[spinner.getSelectedItemPosition()]);
                    showMarkers(amount);
                }

                mapView.addOnMapChangedListener(new MapView.OnMapChangedListener() {
                    @Override
                    public void onMapChanged(@MapView.MapChange int change) {
                        if (change == MapView.REGION_IS_CHANGING || change == MapView.REGION_DID_CHANGE) {
                            if (!mapboxMap.getMarkerViewManager().getMarkerViewAdapters().isEmpty()) {
                                TextView viewCountView = (TextView) findViewById(R.id.countView);
                                viewCountView.setText("ViewCache size " + (mapView.getChildCount() - 5));
                            }
                        }
                    }
                });

                mapboxMap.getMarkerViewManager().setOnMarkerViewClickListener(
                    new MapboxMap.OnMarkerViewClickListener() {
                        @Override
                        public boolean onMarkerClick(
                            @NonNull Marker marker, @NonNull View view, @NonNull MapboxMap.MarkerViewAdapter adapter) {
                            Toast.makeText(
                                BulkMarkerActivity.this,
                                "Hello " + marker.getId(),
                                Toast.LENGTH_SHORT).show();
                            return false;
                        }
                    });
            }
        }
    }

    private static class LoadLocationTask extends AsyncTask<Void, Integer, List<LatLng>> {

        private static final String TAG = "LoadLocationTask";
        private BulkMarkerActivity activity;
        private ProgressDialog progressDialog;
        private int amount;

        public LoadLocationTask(BulkMarkerActivity activity, int amount) {
            this.amount = amount;
            this.activity = activity;
            progressDialog = ProgressDialog.show(activity, "Loading", "Fetching markers", false);
        }

        @Override
        protected List<LatLng> doInBackground(Void... params) {
            try {
                String json = GeoParseUtil.loadStringFromAssets(activity.getApplicationContext(), "points.geojson");
                return GeoParseUtil.parseGeoJSONCoordinates(json);
            } catch (IOException | JSONException exception) {
                Log.e(TAG, "Could not add markers,", exception);
                return null;
            }
        }

        @Override
        protected void onPostExecute(List<LatLng> locations) {
            super.onPostExecute(locations);
            activity.onLatLngListLoaded(locations, amount);
            progressDialog.hide();
        }
    }
}
