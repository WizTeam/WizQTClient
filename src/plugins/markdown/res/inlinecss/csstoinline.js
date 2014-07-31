var cssRules = {
			'propertyGroups' : {
				'block' : ['margin', 'padding', 'background-color', 'border', 'box-shadow', 'border-radius', 'font-size', 'line-height'],
				'inline' : ['color', 'line-height', 'padding', 'margin', 'padding-left'],
				'headings' : ['font-size', 'font-family',],
				'line' : ['padding', 'color', 'margin']
			},
			'elementGroups' : {
				'block' : ['DIV', 'P', 'H1', 'PRE', 'CODE'], 
				'inline' : ['SPAN', 'OL'], 
				'headings' : ['H1', 'H2', 'H3', 'H4', 'H5', 'H6'],
				'line' : ['LI']
			}
		}
$(window).load(function() {
	//$('#before').val( $('body').contents()).html() );
	$('body').inlineStyler( cssRules );
	//$('#after').val( $('boyd').contents()).html() );
});