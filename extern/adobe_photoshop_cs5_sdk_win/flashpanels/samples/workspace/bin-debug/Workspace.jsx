try {
var workspace = new Object;

workspace.addWorkspace = function( inName ) {
	var s = "";
	try {
		var fixedName = workspace.fromActionScript( inName );
		var idMk = charIDToTypeID( "Mk  " );
    	var desc1 = new ActionDescriptor();
    	var idnull = charIDToTypeID( "null" );
	        var ref1 = new ActionReference();
        	var idworkspace = stringIDToTypeID( "workspace" );
        	ref1.putClass( idworkspace );
    	desc1.putReference( idnull, ref1 );
    	var idUsng = charIDToTypeID( "Usng" );
	        var desc2 = new ActionDescriptor();
        	var idNm = charIDToTypeID( "Nm  " );
        	desc2.putString( idNm, fixedName );
        	var idPlt = charIDToTypeID( "Plt " );
        	desc2.putBoolean( idPlt, true );
        	var idkeyboardCustomization = stringIDToTypeID( "keyboardCustomization" );
        	desc2.putBoolean( idkeyboardCustomization, false );
	        var idmenuCustomization = stringIDToTypeID( "menuCustomization" );
        	desc2.putBoolean( idmenuCustomization, false );
        	var idreplace = stringIDToTypeID( "replace" );
        	desc2.putBoolean( idreplace, false );
    	var idworkspace = stringIDToTypeID( "workspace" );
    	desc1.putObject( idUsng, idworkspace, desc2 );
		executeAction( idMk, desc1, DialogModes.NO );
		s = workspace.stringToXML("OK");
	}
	catch(e) {
		s = workspace.errorToXML(e);
	}
	return s;
}

workspace.deleteWorkspace = function( inName ) {
	var s = "";
	try {
		var fixedName = workspace.fromActionScript( inName );
		var idDlt = charIDToTypeID( "Dlt " );
	    var desc3 = new ActionDescriptor();
	    var idnull = charIDToTypeID( "null" );
        	var ref2 = new ActionReference();
        	var idworkspace = stringIDToTypeID( "workspace" );
        	ref2.putName( idworkspace, fixedName );
    	desc3.putReference( idnull, ref2 );
		executeAction( idDlt, desc3, DialogModes.NO );
		s = workspace.stringToXML("OK");
	}
	catch(e) {
		s = workspace.errorToXML(e);
	}
	return s;
}

workspace.selectWorkspace = function( inName ) {
	var s = "";
	try {
		var fixedName = workspace.fromActionScript( inName );
		var idslct = charIDToTypeID( "slct" );
    	var desc4 = new ActionDescriptor();
    	var idnull = charIDToTypeID( "null" );
	        var ref4 = new ActionReference();
        	var idworkspace = stringIDToTypeID( "workspace" );
        	ref4.putName( idworkspace, fixedName );
    	desc4.putReference( idnull, ref4 );
		executeAction( idslct, desc4, DialogModes.NO );
		s = workspace.stringToXML("OK");
	}
	catch(e) {
		s = workspace.errorToXML(e);
	}
	return s;
}

workspace.resetWorkspace = function( inName ) {
	var s = "";
	try {
		var fixedName = workspace.fromActionScript( inName );
		var idRset = charIDToTypeID( "Rset" );
   	 	var desc4 = new ActionDescriptor();
   	 	var idnull = charIDToTypeID( "null" );	
   	    	var ref3 = new ActionReference();
   	    	var idworkspace = stringIDToTypeID( "workspace" );
   	    	ref3.putName( idworkspace, fixedName );
   		desc4.putReference( idnull, ref3 );
		executeAction( idRset, desc4, DialogModes.NO );
		s = workspace.stringToXML("OK");
	}
	catch(e) {
		s = workspace.errorToXML(e);
	}
	return s;
}

workspace.getWorkspaceNames = function () {
	var s = "";
	try {
		var theNames = new Array();
    	var keyName = charIDToTypeID( "Nm  " );
    	var displayName = stringIDToTypeID( "displayName" );
    	var workspaceList = stringIDToTypeID( "workspaceList" );
    	var typeOrdinal = charIDToTypeID( "Ordn" );
    	var enumTarget = charIDToTypeID( "Trgt" );
    	var classProperty = charIDToTypeID( "Prpr" );
    	var classApplication = charIDToTypeID( "capp" );
    	var ref = new ActionReference();
    	ref.putProperty( classProperty, workspaceList );
    	ref.putEnumerated( classApplication, typeOrdinal, enumTarget );
    	var desc = executeActionGet( ref );
    	var wsList = desc.getList( workspaceList );
    	for (var i = 0; i < wsList.count; i++) {
	    	var wsDesc = wsList.getObjectValue( i );
	    	var dName = wsDesc.getString( displayName ); 
	    	theNames.push( dName );
    	}
	    s = workspace.stringToXML(theNames.toString());
	}
	catch(e) {
		alert(e + ":" + e.line);
		s = workspace.errorToXML(e);
	}
	return s;
}

workspace.fromActionScript = function ( inActionScript ) {
	return decodeURIComponent( unescape( inActionScript ) );
}

///////////////////////////////////////////////////////////////////////////////
// Function: stringToXML
// Usage: Returns to the panel must be wrapped in XML syntax
// Input: inString string to wrap in XML
// Return: string in XML syntax
///////////////////////////////////////////////////////////////////////////////
workspace.stringToXML = function(inString) {
    var s = '<object>';
    s += workspace.propertyToXML(0, "error");
    s += workspace.propertyToXML(inString, "message");
    s += '</object>';
    return s;
}

///////////////////////////////////////////////////////////////////////////////
// Function: errorToXML
// Usage: Returns to the panel must be wrapped in XML syntax
// Input: inError is an Error object
// Return: Error in XML syntax
///////////////////////////////////////////////////////////////////////////////
workspace.errorToXML = function(inError) {
    var s = '<object>';
    s += workspace.propertyToXML(inError.number, "error");
    s += workspace.propertyToXML(inError.message, "message");
    s += workspace.propertyToXML(inError.line, "line");
    s += workspace.propertyToXML(inError.fileName, "fileName");
    s += '</object>';
    return s;
}

///////////////////////////////////////////////////////////////////////////////
// Function: propertyToXML
// Usage: Returns to the panel must be wrapped in XML syntax, helper routine
// Input: inProperty property string
//        inID property id
// Return: property wrapped in XML syntax for return value to the panel
///////////////////////////////////////////////////////////////////////////////
workspace.propertyToXML = function(inProperty, inID) {
    var t = typeof inProperty;
    var s = '<property id="' + inID + '">';
    switch (t) {
        case "number":
            s += '<number>' + inProperty.toString() + '</number>';
            break;
        case "boolean":
            s += '<' + inProperty.toString() + '/>';
            break;
        case "string":
            s += '<string>' + inProperty.toString() + '</string>';
            break;
        // TODO figure this out
        case "object":
            //s += '<object>';
			//for (var i in inProperty) {
			//	s += propertyToXML(inProperty[i], i.toString());
			//}
			//s += '</object>';
            break;
        case "undefined":
            s += '<string>undefined</string>';
            break;
        default:
            if (workspace.debugAlerts) alert('unknown type not supported: ' + t);
            break;
    }
    s += '</property>';
    return s;
}
}
catch(e) {
	alert(e + ":" + e.line);
}

